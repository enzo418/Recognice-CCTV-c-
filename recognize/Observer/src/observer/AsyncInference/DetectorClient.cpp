#include "DetectorClient.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

#include "internal/eventing/epoll_kqueue.h"
#include "libusockets.h"
#include "observer/Implementation.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Semaphore.hpp"
#include "observer/Timer.hpp"
#include "observer/Utils/SpecialEnums.hpp"
#include "types.hpp"

namespace AsyncInference {
    static const int SSL = 0;
    static const char* classes[] = {
        "person",        "bicycle",       "car",           "motorbike",
        "aeroplane",     "bus",           "train",         "truck",
        "boat",          "traffic light", "fire hydrant",  "stop sign",
        "parking meter", "bench",         "bird",          "cat",
        "dog",           "horse",         "sheep",         "cow",
        "elephant",      "bear",          "zebra",         "giraffe",
        "backpack",      "umbrella",      "handbag",       "tie",
        "suitcase",      "frisbee",       "skis",          "snowboard",
        "sports ball",   "kite",          "baseball bat",  "baseball glove",
        "skateboard",    "surfboard",     "tennis racket", "bottle",
        "wine glass",    "cup",           "fork",          "knife",
        "spoon",         "bowl",          "banana",        "apple",
        "sandwich",      "orange",        "broccoli",      "carrot",
        "hot dog",       "pizza",         "donut",         "cake",
        "chair",         "sofa",          "pottedplant",   "bed",
        "diningtable",   "toilet",        "tvmonitor",     "laptop",
        "mouse",         "remote",        "keyboard",      "cell phone",
        "microwave",     "oven",          "toaster",       "sink",
        "refrigerator",  "book",          "clock",         "vase",
        "scissors",      "teddy bear",    "hair drier",    "toothbrush",
    };

    /* ------------------------------------------------------ */
    /*               SendEveryNthFrame Strategy               */
    /* ------------------------------------------------------ */
    SendEveryNthFrame::SendEveryNthFrame(int n) {
        if (n <= 0) {
            throw std::invalid_argument("n must be greater than 0");
        }

        this->n = n;
    }

    bool SendEveryNthFrame::ShouldSend(Observer::Frame&, int index) {
        return index % n == 0;
    }

    /* ------------------------------------------------------ */
    /*                      DetectCaller                      */
    /* ------------------------------------------------------ */

    void UpdateImageDetections(
        DetectorClient& client, int image_index, int remaining_boxes,
        const std::optional<SingleDetection>& detection) {
        client.UpdateImageDetections(image_index, remaining_boxes, detection);
    }

    void DetectorClient::UpdateImageDetections(
        int image_index, int remaining_boxes,
        const std::optional<SingleDetection>& detection) {
        if (image_results.contains(image_index)) {
            auto& container = image_results[image_index];
            if (container.all_results_received) {
                OBSERVER_WARN("Received detection for completed image: {}",
                              image_index);
                return;
            }

            // printf("Received detection for image %d: Class=%s Prob=%f\n",
            //        image_index, detection.label.c_str(),
            //        detection.confidence);

            if (remaining_boxes == 0) {
                container.all_results_received = true;
            }
            if (detection.has_value()) {
                container.results.push_back(detection.value());

                // flash the results to the caller thread
                singleResultsMutex.lock();
                assert(detection.value().label.size() > 0);
                singleResults[currentSingleResultsContainer].push_back(
                    detection.value());
                assert(singleResults[currentSingleResultsContainer]
                           .back()
                           .label.size() > 0);
                singleResultsMutex.unlock();
            }

            // check if all results are received
            for (auto& [index, result] : image_results) {
                if (!result.all_results_received) {
                    // printf(
                    //     "Group %d Image %d has boxes remaining, received
                    //     %lu\n", result.group_number, index,
                    //     result.results.size());
                    return;
                }
            }

            detectionDoneFlag = true;
        } else {
            OBSERVER_WARN("Received detection for unknown image: {}",
                          image_index);
            return;
        }
    }

    int us_socket_write_or_backpressure(int SSL, us_socket_t* s, char* data,
                                        int length, int msg_more) {
        detector_socket* ds = (detector_socket*)us_socket_ext(SSL, s);

        int written = 0;

        // if we have backpressure, we need to write that first before the new
        // data
        if (ds->length == 0) {
            written = us_socket_write(SSL, s, data, length, msg_more);
        }

        if (written != length) {
            char* new_buffer = (char*)malloc(ds->length + length - written);

            if (!new_buffer) {
                // what's the point anymore
                OBSERVER_CRITICAL("Failed to allocate memory for backpressure");
                return written;
            }

            if (ds->length > 0 && ds->backpressure) {
                memcpy(new_buffer, ds->backpressure, ds->length);
                free(ds->backpressure);
            }

            memcpy(new_buffer + ds->length, data + written, length - written);
            ds->backpressure = new_buffer;
            ds->length += length - written;
        }

        return written;
    }

    struct us_socket_t* on_data(us_socket_t* s, char* data, int length) {
        // printf("Received data: %d bytes\n", length);

        /* Do not accept any data while in shutdown state */
        if (us_socket_is_shut_down(SSL, (us_socket_t*)s)) {
            return s;
        }

        detector_socket* ds = (detector_socket*)us_socket_ext(SSL, s);

        detector_context* ctx = (detector_context*)us_socket_context_ext(
            SSL, us_socket_context(SSL, s));

        auto& header = ds->read_ctx.header;
        bool& header_received = ds->read_ctx.header_received;

        int bytes_left = length;
        while (bytes_left > 0) {
            if (!header_received) {
                auto& bytes_received = ds->read_ctx.header_bytes_received;

                size_t bytes_to_copy =
                    std::min(sizeof(DetectionResultHeader) - bytes_received,
                             (size_t)bytes_left);

                memcpy((char*)&header + bytes_received, data, bytes_to_copy);

                bytes_received += bytes_to_copy;
                data += bytes_to_copy;
                bytes_left -= bytes_to_copy;

                if (bytes_received == sizeof(DetectionResultHeader)) {
                    bytes_received = 0;

                    // printf("Received header: Group=%d Image=%d Boxes=%d\n",
                    //        header.group_number, header.image_number,
                    //        header.num_boxes);
                    if (header.num_boxes == 0) {
                        header_received = false;

                        UpdateImageDetections(*ctx->client, header.image_number,
                                              0, std::nullopt);
                    } else {
                        header_received = true;
                    }
                }
            } else {
                auto& bytes_received = ds->read_ctx.box_bytes_received;
                auto& box = ds->read_ctx.box;

                size_t bytes_to_copy =
                    std::min(sizeof(DetectionBoxData) - bytes_received,
                             (size_t)bytes_left);

                memcpy((char*)&box + bytes_received, data, bytes_to_copy);

                bytes_received += bytes_to_copy;
                data += bytes_to_copy;
                bytes_left -= bytes_to_copy;

                if (bytes_received == sizeof(DetectionBoxData)) {
                    bytes_received = 0;

                    ds->read_ctx.num_boxes_received++;
                    std::string class_label = "??";

                    if (box.class_id < sizeof(classes) / sizeof(classes[0])) {
                        class_label = classes[box.class_id];
                    }

                    // printf("\tReceived box: Class=%d Prob=%f\n",
                    // box.class_id,
                    //        box.prob);
                    UpdateImageDetections(
                        *ctx->client, header.image_number,
                        header.num_boxes - ds->read_ctx.num_boxes_received,
                        SingleDetection {
                            .x = box.x,
                            .y = box.y,
                            .width = box.w,
                            .height = box.h,
                            .confidence = box.prob,
                            .label = class_label,
                        });

                    if (ds->read_ctx.num_boxes_received == header.num_boxes) {
                        header_received = false;
                        ds->read_ctx.num_boxes_received = 0;
                    }
                }
            }
        }

        return s;
    }

    void DetectorClient::OnError() {
        OBSERVER_ERROR("Connection error");
        this->socket_status = SocketStatus::ERROR;
        this->socketConnectionSmp.release();
    }

    void DetectorClient::OnOpen(struct us_socket_t* socket) {
        this->socket = socket;
        this->socket_status = SocketStatus::CONNECTED;
        socketConnectionSmp.release();
    }

    void OnOpen(DetectorClient& client, struct us_socket_t* socket) {
        client.OnOpen(socket);
    }

    void OnError(DetectorClient& client) { client.OnError(); }

    struct us_socket_t* on_socket_conn_error(struct us_socket_t* s, int) {
        detector_context* ctx = (detector_context*)us_socket_context_ext(
            SSL, us_socket_context(SSL, s));

        OnError(*ctx->client);

        return s;
    }

    struct us_socket_t* on_socket_open(struct us_socket_t* s, int, char*, int) {
        struct detector_socket* ds =
            (struct detector_socket*)us_socket_ext(SSL, s);

        detector_context* ctx = (detector_context*)us_socket_context_ext(
            SSL, us_socket_context(SSL, s));

        ds->backpressure = 0;
        ds->length = 0;

        ds->read_ctx.header_received = false;
        ds->read_ctx.header_bytes_received = 0;
        ds->read_ctx.num_boxes_received = 0;
        ds->read_ctx.box_bytes_received = 0;

        OnOpen(*ctx->client, s);

        return s;
    }

    void DetectorClient::OnClose() {
        this->socket_status = SocketStatus::DISCONNECTED;
        socketConnectionSmp.release();
    }

    void OnClose(DetectorClient& client) { client.OnClose(); }

    struct us_socket_t* on_socket_close(struct us_socket_t* s, int, void*) {
        struct detector_socket* ds =
            (struct detector_socket*)us_socket_ext(SSL, s);

        detector_context* ctx = (detector_context*)us_socket_context_ext(
            SSL, us_socket_context(SSL, s));

        OnClose(*ctx->client);

        OBSERVER_TRACE("Client disconnected");

        free(ds->backpressure);

        return s;
    }

    /* Socket half-closed handler */
    struct us_socket_t* on_socket_end(struct us_socket_t* s) {
        us_socket_shutdown(SSL, s);
        return us_socket_close(SSL, s, 0, NULL);
    }

    struct us_socket_t* on_socket_timeout(struct us_socket_t* s) {
        /* Close idle HTTP sockets */
        return us_socket_close(SSL, s, 0, NULL);
    }

    struct us_socket_t* on_detect_socket_writable(struct us_socket_t* s) {
        struct detector_socket* ds =
            (struct detector_socket*)us_socket_ext(SSL, s);

        int written = us_socket_write(SSL, s, ds->backpressure, ds->length, 0);
        if (written != ds->length) {
            char* new_buffer = (char*)malloc(ds->length - written);

            if (ds->length > 0 && ds->backpressure) {
                memcpy(new_buffer, ds->backpressure + written,
                       ds->length - written);
                free(ds->backpressure);
            }

            ds->backpressure = new_buffer;
            ds->length -= written;
        } else {
            free(ds->backpressure);
            ds->backpressure = nullptr;
            ds->length = 0;
        }

        return s;
    }

    void dummy(struct us_loop_t*) {}

    void wakeup(struct us_loop_t* loop) {
        loop_context* loopData = (loop_context*)us_loop_ext(loop);

        /* Swap current deferQueue */
        loopData->deferMutex.lock();
        int oldDeferQueue = loopData->currentDeferQueue;
        loopData->currentDeferQueue = (loopData->currentDeferQueue + 1) % 2;
        loopData->deferMutex.unlock();

        /* Drain the queue */
        for (auto& x : loopData->deferQueues[oldDeferQueue]) {
            x();
        }
        loopData->deferQueues[oldDeferQueue].clear();
    }

    void defer(struct us_loop_t* loop, std::function<void()>&& cb) {
        loop_context* loopData = (loop_context*)us_loop_ext((us_loop_t*)loop);

        loopData->deferMutex.lock();
        loopData->deferQueues[loopData->currentDeferQueue].emplace_back(
            std::move(cb));
        loopData->deferMutex.unlock();

        us_wakeup_loop((us_loop_t*)loop);
    }

    DetectorClient::DetectorClient(const std::string& server_address)
        : serverAddress(server_address) {
        this->socket_status = SocketStatus::DISCONNECTED;

        /* ----------------- Initialize uSockets ---------------- */
        this->loop =
            us_create_loop(0, wakeup, dummy, dummy, sizeof(loop_context));

        new (us_loop_ext((us_loop_t*)loop)) loop_context;

        this->us_context = us_create_socket_context(
            SSL, loop, sizeof(struct detector_context), {});

        auto context =
            (detector_context*)us_socket_context_ext(SSL, us_context);
        context->client = this;

        // // Set up callbacks
        us_socket_context_on_open(SSL, us_context, on_socket_open);
        us_socket_context_on_data(SSL, us_context, on_data);
        us_socket_context_on_close(SSL, us_context, on_socket_close);
        us_socket_context_on_end(SSL, us_context, on_socket_end);
        us_socket_context_on_timeout(SSL, us_context, on_socket_timeout);
        us_socket_context_on_connect_error(SSL, us_context,
                                           on_socket_conn_error);

        us_socket_context_on_writable(SSL, us_context,
                                      on_detect_socket_writable);
    }

    DetectorClient::~DetectorClient() {
        this->Stop();

        if (loopThread.joinable()) {
            loopThread.join();
        }

        us_socket_context_free(SSL, this->us_context);

        us_loop_free(loop);
        loop_context* loop_ctx = (loop_context*)us_loop_ext((us_loop_t*)loop);
        loop_ctx->~loop_context();
    }

    void DetectorClient::Disconnect() {
        if (loopThread.joinable()) {
            // socket is alive

            if (socket &&
                !Observer::has_flag(this->socket_status,
                                    SocketStatus::DISCONNECTED) &&
                !Observer::has_flag(this->socket_status, SocketStatus::ERROR)) {
                if (!us_socket_is_closed(SSL, socket) &&
                    !us_socket_is_shut_down(SSL, socket)) {
                    if (Observer::has_flag(this->socket_status,
                                           SocketStatus::CONNECTED))
                        us_socket_shutdown(SSL, socket);
                    else if (Observer::has_flag(this->socket_status,
                                                SocketStatus::CONNECTING))
                        us_socket_close_connecting(SSL, socket);

                    this->socket_status = SocketStatus::DISCONNECTED;
                }
            }

            loop_running = false;

            if (loopThread.joinable()) {
                loopThread.join();
            }
        } else {
            if (!Observer::has_flag(this->socket_status,
                                    SocketStatus::DISCONNECTED)) {
                OBSERVER_WARN(
                    "Loop finished and freed the socket before it was closed. "
                    "Socket status {}",
                    (int)this->socket_status);
            }
            this->socket_status = SocketStatus::DISCONNECTED;
        }
    }

    bool DetectorClient::Connect() {
        if (socket != nullptr &&
            Observer::has_flag(this->socket_status, SocketStatus::CONNECTED)) {
            char ip[20];
            int length {0};
            us_socket_remote_address(SSL, socket, ip, &length);
            on_socket_open(socket, 1, ip, length);
        } else if (!Observer::has_flag(this->socket_status,
                                       SocketStatus::CONNECTING)) {
            if (serverAddress.find(':') == std::string::npos) {
                OBSERVER_ERROR("Invalid server address: {}", serverAddress);
                return false;
            }

            std::string host = serverAddress.substr(0, serverAddress.find(':'));
            int port =
                std::stoi(serverAddress.substr(serverAddress.find(':') + 1));

            this->socket = us_socket_context_connect(
                SSL, us_context, host.data(), port, NULL, 0,
                sizeof(struct detector_socket));

            OBSERVER_ASSERT(this->socket, "No socket");

            us_socket_timeout(SSL, (us_socket_t*)this->socket, 4);

            this->socket_status = SocketStatus::CONNECTING;

            if (!loop_running) {
                loopThread = std::thread(us_loop_run, loop);
                loop_running = true;
            }
        }

        if (!socketConnectionSmp.acquire_timeout<10>()) {
            OBSERVER_TRACE("Waiting for socket to open");

            Observer::Timer<std::chrono::seconds> timer(true);
            while (!socketConnectionSmp.acquire_timeout<50>() && !stopFlag) {
                if (timer.GetDuration() > 5) {
                    OBSERVER_ERROR("Timeout while waiting for socket to open");

                    if (Observer::has_flag(this->socket_status,
                                           SocketStatus::CONNECTING)) {
                        us_socket_close_connecting(SSL, socket);
                        this->socket_status = SocketStatus::DISCONNECTED;
                    }

                    return false;
                }
            }
        }

        if (!Observer::has_flag(this->socket_status, SocketStatus::CONNECTED) ||
            Observer::has_flag(this->socket_status, SocketStatus::ERROR)) {
            if (loopThread.joinable()) loopThread.join();
            loop_running = false;
        } else {
            us_socket_timeout(SSL, (us_socket_t*)this->socket, 0);
        }

        return Observer::has_flag(this->socket_status,
                                  SocketStatus::CONNECTED) &&
               !Observer::has_flag(this->socket_status, SocketStatus::ERROR);
    }

    std::vector<std::string> DetectorClient::GetModelNames() { return {}; }

    void DetectorClient::WriteImages(std::vector<Observer::Frame>& images) {
        static std::atomic_uint32_t s_group_number = 0;
        uint32_t group_number = s_group_number++;

        OBSERVER_SCOPE("Detector send images");

        image_results.clear();

        int sending = 0;

        for (size_t i = 0; i < images.size(); i++) {
            auto& image = images[i];

            if (stopFlag) {
                return;
            }

            if ((sendStrategy != nullptr &&
                 !sendStrategy->ShouldSend(image, i)) ||
                image.IsEmpty()) {
                continue;
            }

            buffer.clear();

            // Create a packet header
            PacketHeader header;
            header.version = 1;
            header.image_number = i;
            header.group_number = group_number;
            header.width = image.GetSize().width;
            header.height = image.GetSize().height;
            header.image_type = ImageType::ENCODED;
            header.padding = 0;

            if (!image.EncodeImage(".jpg", 95, buffer)) {
                OBSERVER_ERROR("Failed to encode image {}", i);
                return;
            }

            image_results[header.image_number] = {
                .results = {},
                .all_results_received = false,
                .group_number = group_number,
            };

            header.data_length = buffer.size();

            char* bufcp = (char*)malloc(buffer.size());
            if (bufcp == nullptr) {
                OBSERVER_ERROR("Failed to allocate memory for image buffer");
                return;
            }

            memcpy(bufcp, buffer.data(), buffer.size());

            defer(this->loop, [this, header, bufcp]() {
                // Send the header
                us_socket_write_or_backpressure(SSL, socket, (char*)&header,
                                                sizeof(PacketHeader), 1);

                // Send the image
                us_socket_write_or_backpressure(SSL, socket, bufcp,
                                                header.data_length, 0);

                free(bufcp);
            });

            sending++;
        }

        OBSERVER_TRACE("Detector will send {} images in group {}", sending,
                       group_number);
    }

    std::vector<ImageDetections> DetectorClient::Detect(
        std::vector<Observer::Frame>& pImages,
        std::function<void(const SingleDetection&)>&& pOnResult,
        ISendStrategy* pSendStrategy) {
        OBSERVER_ASSERT(
            Observer::has_flag(this->socket_status, SocketStatus::CONNECTED),
            "Socket not connected");
        OBSERVER_ASSERT(
            !Observer::has_flag(this->socket_status, SocketStatus::ERROR),
            "Socket in error state");

        this->onResult = std::move(pOnResult);
        this->sendStrategy = pSendStrategy;
        WriteImages(pImages);

        SingleDetection detection;
        while (!stopFlag && !detectionDoneFlag) {
            singleResultsMutex.lock();
            int oldSingleResultsContainer = currentSingleResultsContainer;
            currentSingleResultsContainer =
                (currentSingleResultsContainer + 1) % 2;
            singleResultsMutex.unlock();

            for (auto& detection : singleResults[oldSingleResultsContainer]) {
                this->onResult(detection);
            }
            singleResults[oldSingleResultsContainer].clear();

            // check if connection status changed
            if (socketConnectionSmp.acquire_timeout<10>() &&
                !Observer::has_flag(this->socket_status,
                                    SocketStatus::CONNECTED)) {
                OBSERVER_TRACE("Ending detection: connection was lost.");
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::vector<ImageDetections> all_results;
        for (auto& [i, result] : image_results) {
            ImageDetections image_result;
            image_result.image_index = i;
            image_result.detections = result.results;
            all_results.push_back(image_result);
        }

        detectionDoneFlag = false;
        stopFlag = false;

        return all_results;
    }

    void DetectorClient::SetServerAddress(const std::string& address) {
        this->serverAddress = address;
    }

    void DetectorClient::Stop() {
        stopFlag = true;

        this->Disconnect();
    }

}  // namespace AsyncInference
