#include "DetectorClient.hpp"

#include <atomic>
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
#include "observer/Utils/SpecialEnums.hpp"
#include "types.hpp"

namespace AsyncInference {
    static struct us_loop_t* g_loop = nullptr;
    static std::atomic_bool g_loop_running = false;
    static std::thread g_loopThread;
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

    void AddResult(DetectorClient& client, const SingleDetection& detection,
                   int image_index, int remaining_boxes) {
        client.AddResult(detection, image_index, remaining_boxes);
    }

    void DetectorClient::AddResult(const SingleDetection& detection,
                                   int image_index, int remaining_boxes) {
        if (image_results.contains(image_index)) {
            auto& container = image_results[image_index];
            if (container.all_results_received) {
                OBSERVER_WARN("Received detection for completed image: {}",
                              image_index);
                return;
            }

            printf("Received detection for image %d: Class=%s Prob=%f\n",
                   image_index, detection.label.c_str(), detection.confidence);

            container.results.push_back(detection);

            if (remaining_boxes == 0) {
                container.all_results_received = true;
            }

            // flash the results to the caller thread
            singleResultsMutex.lock();
            assert(detection.label.size() > 0);
            singleResults[currentSingleResultsContainer].push_back(detection);
            assert(singleResults[currentSingleResultsContainer]
                       .back()
                       .label.size() > 0);
            singleResultsMutex.unlock();

            // check if all results are received
            for (auto& [index, result] : image_results) {
                if (!result.all_results_received) {
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

                    // printf("Received header: Image=%d Boxes=%d\n",
                    //        header.image_number, header.num_boxes);
                    if (header.num_boxes == 0) {
                        header_received = false;
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
                    AddResult(
                        *ctx->client,
                        SingleDetection {
                            .x = box.x,
                            .y = box.y,
                            .width = box.w,
                            .height = box.h,
                            .confidence = box.prob,
                            .label = class_label,
                        },
                        header.image_number,
                        header.num_boxes - ds->read_ctx.num_boxes_received);

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

    struct us_socket_t* on_socket_open(struct us_socket_t* s, int is_client,
                                       char* ip, int ip_length) {
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
    }

    void OnClose(DetectorClient& client) { client.OnClose(); }

    struct us_socket_t* on_socket_close(struct us_socket_t* s, int code,
                                        void* reason) {
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
            memcpy(new_buffer, ds->backpressure, ds->length - written);
            free(ds->backpressure);
            ds->backpressure = new_buffer;
            ds->length -= written;
        } else {
            free(ds->backpressure);
            ds->length = 0;
        }

        return s;
    }

    void dummy(struct us_loop_t*) {}

    void wakeup(struct us_loop_t*) {
        loop_context* loopData = (loop_context*)us_loop_ext(g_loop);

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

    void defer(std::function<void()>&& cb) {
        loop_context* loopData = (loop_context*)us_loop_ext((us_loop_t*)g_loop);

        loopData->deferMutex.lock();
        loopData->deferQueues[loopData->currentDeferQueue].emplace_back(
            std::move(cb));
        loopData->deferMutex.unlock();

        us_wakeup_loop((us_loop_t*)g_loop);
    }

    DetectorClient::DetectorClient(const std::string& server_address)
        : serverAddress(server_address) {
        this->socket_status = SocketStatus::DISCONNECTED;

        /* ----------------- Initialize uSockets ---------------- */
        if (!g_loop) {
            g_loop =
                us_create_loop(0, wakeup, dummy, dummy, sizeof(loop_context));

            new (us_loop_ext((us_loop_t*)g_loop)) loop_context;
        }

        this->us_context = us_create_socket_context(
            SSL, g_loop, sizeof(struct detector_context), {});

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

        if (g_loopThread.joinable()) {
            g_loopThread.join();
        }

        OBSERVER_ASSERT(g_loop, "Loop not created");

        us_socket_context_free(SSL, us_socket_context(SSL, this->socket));
        us_loop_free(g_loop);
        loop_context* loop_ctx = (loop_context*)us_loop_ext((us_loop_t*)g_loop);
        loop_ctx->~loop_context();

        g_loop = nullptr;
    }

    void DetectorClient::Disconnect() {
        if (socket && !us_socket_is_closed(SSL, socket) &&
            !us_socket_is_shut_down(SSL, socket)) {
            us_socket_close_connecting(SSL, socket);
        }

        this->socket_status = SocketStatus::DISCONNECTED;

        g_loop_running = false;

        if (g_loopThread.joinable()) {
            g_loopThread.join();
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
            std::string host = serverAddress.substr(0, serverAddress.find(':'));
            int port =
                std::stoi(serverAddress.substr(serverAddress.find(':') + 1));

            this->socket = us_socket_context_connect(
                SSL, us_context, host.data(), port, NULL, 0,
                sizeof(struct detector_socket));

            this->socket_status = SocketStatus::CONNECTING;

            if (!g_loop_running) {
                g_loopThread = std::thread(us_loop_run, g_loop);
                g_loop_running = true;
            }
        }

        while (!socketConnectionSmp.acquire_timeout<10>() && !stopFlag) {
            OBSERVER_TRACE("Waiting for socket to open");
        }

        if (!Observer::has_flag(this->socket_status, SocketStatus::CONNECTED) ||
            Observer::has_flag(this->socket_status, SocketStatus::ERROR)) {
            if (g_loopThread.joinable()) g_loopThread.join();
            g_loop_running = false;
        }

        return Observer::has_flag(this->socket_status,
                                  SocketStatus::CONNECTED) &&
               !Observer::has_flag(this->socket_status, SocketStatus::ERROR);
    }

    std::vector<std::string> DetectorClient::GetModelNames() { return {}; }

    void DetectorClient::WriteImages(std::vector<Observer::Frame>& images) {
        static std::atomic_uint32_t group_number = 0;

        group_number++;

        OBSERVER_SCOPE("Detector send images");

        image_results.clear();

        for (size_t i = 0; i < images.size(); i++) {
            auto& image = images[i];

            if (stopFlag) {
                std::cout << "DetectorClient stopped." << std::endl;
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
                std::cout << "DetectorClient failed to encode image."
                          << std::endl;
                return;
            }

            image_results[header.image_number] = {
                .results = {},
                .all_results_received = false,
                .callback_called = false,
            };

            header.data_length = buffer.size();

            char* bufcp = (char*)malloc(buffer.size());
            if (bufcp == nullptr) {
                OBSERVER_ERROR("Failed to allocate memory for image buffer");
                return;
            }

            memcpy(bufcp, buffer.data(), buffer.size());

            defer([this, header, bufcp]() {
                // Send the header
                us_socket_write(SSL, socket, (char*)&header,
                                sizeof(PacketHeader), 1);

                // Send the image
                us_socket_write(SSL, socket, bufcp, header.data_length, 0);

                free(bufcp);
            });
        }
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

    void DetectorClient::Stop() {
        stopFlag = true;

        this->Disconnect();
    }

}  // namespace AsyncInference