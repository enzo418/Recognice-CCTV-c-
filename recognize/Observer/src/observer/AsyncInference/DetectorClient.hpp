#pragma once

#include <libusockets.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string_view>
#include <thread>

#include "observer/IFrame.hpp"
#include "observer/Semaphore.hpp"
#include "observer/SimpleBlockingQueue.hpp"
#include "opencv2/opencv.hpp"
#include "types.hpp"

namespace AsyncInference {

    class DetectCaller;

    // Define the packet header structure
    enum ImageType { RAW_BGR, RAW_RGB, ENCODED };  // Assumed with 3 channels

#pragma pack(1)
    struct PacketHeader {
        uint8_t version;
        uint16_t image_number;  // Image number in the group
        uint32_t group_number;  // Group number
        uint32_t data_length;   // Length of the image data
        uint8_t image_type;     // New field for image type
        uint16_t width;         // New field for image width
        uint16_t height;        // New field for image height
        uint32_t padding {0};
    };

    struct DetectionBoxData {
        uint16_t class_id;  // based on coco.names
        float prob;
        float x, y, w, h;

        uint16_t padding;
    };

    struct DetectionResultHeader {
        uint32_t group_number;
        uint16_t image_number;
        uint16_t num_boxes;
    };
#pragma pack()

    struct socket_read_ctx {
        DetectionResultHeader* header {nullptr};
        char* buffer_read;
        int length_read;
    };

    struct detector_context {
        class DetectorClient* client;
    };
    struct socket_read_context {
        DetectionResultHeader header;
        bool header_received {false};
        size_t header_bytes_received {0};

        DetectionBoxData box;
        size_t box_bytes_received {0};

        uint16_t num_boxes_received {0};
    };

    struct detector_socket {
        char* backpressure {nullptr};
        int length {0};

        socket_read_context read_ctx;
    };

    struct loop_context {
        std::mutex deferMutex;
        int currentDeferQueue = 0;
        std::vector<std::function<void()>> deferQueues[2];
    };

    enum class SocketStatus {
        CONNECTED = 1,
        DISCONNECTED = 2,
        ERROR = 4,
        CONNECTING = 8
    };

    class DetectorClient {
       public:
        struct ISendStrategy {
            virtual bool ShouldSend(Observer::Frame& frame, int index) = 0;
        };

       public:
        DetectorClient(const std::string& server_address);
        ~DetectorClient();

       public:
        std::vector<std::string> GetModelNames();

        /**
         * @brief Detects objects in the given image.
         * Blocks the calling thread until the client is disconnected or all the
         * results are received.
         *
         * @param images images to detect objects in.
         * @param onResult callback function that is called when a result is
         * received from the server. Is not guaranteed to be called with all the
         * results, all the results are stored in the return value.
         * @param sendStrategy strategy that is used to send images to the
         * server. If null, all the images will be sent.
         * @return std::vector<ImageDetections> results received from the server
         */
        std::vector<ImageDetections> Detect(
            std::vector<Observer::Frame>& images,
            std::function<void(const SingleDetection&)>&& onResult,
            ISendStrategy* sendStrategy = nullptr);

        /**
         * @brief Stops whatever the client is doing.
         * This function is called automatically when the client is destroyed.
         * Leaves the client in a state where IT CAN be used again.
         * It's safe to call this function multiple times.
         */
        void Stop();

        /**
         * @brief Try to connect to the server.
         * Should call this before `Detect`.
         *
         * @return true true if connected successfully
         * @return false
         */
        bool Connect();
        void Disconnect();

       private:
        Semaphore socketConnectionSmp;

        void OnOpen(struct us_socket_t* socket);
        friend void OnOpen(DetectorClient& client, struct us_socket_t* socket);

        void OnClose();
        friend void OnClose(DetectorClient& client);

        void OnError();
        friend void OnError(DetectorClient& client);

       private:
        void WriteImages(std::vector<Observer::Frame>& images);
        void AddResult(const SingleDetection& detection, int image_index,
                       int remaining_boxes);
        friend void AddResult(DetectorClient& client,
                              const SingleDetection& detection, int image_index,
                              int remaining_boxes);

        std::function<void(const SingleDetection&)> onResult;
        ISendStrategy* sendStrategy;
        std::vector<uchar> buffer;

       private:
        std::string serverAddress;
        std::thread readerThread;
        std::atomic_bool stopFlag {false};

        struct us_socket_t* socket {nullptr};
        us_socket_context_t* us_context;
        SocketStatus socket_status;

        struct ResultHolder {
            std::vector<SingleDetection> results;
            bool all_results_received {false};
            bool callback_called {false};
        };

        // only the event loop thread will access this until detectionDoneFlag
        // is set
        std::unordered_map<int, ResultHolder> image_results;

        std::mutex singleResultsMutex;
        int currentSingleResultsContainer = 0;
        std::vector<SingleDetection> singleResults[2];

        std::atomic_bool detectionDoneFlag {false};
    };

    // Send every nth frame strategy
    class SendEveryNthFrame : public DetectorClient::ISendStrategy {
       public:
        SendEveryNthFrame(int n);
        bool ShouldSend(Observer::Frame& frame, int index) override;

       private:
        int n;
    };

}  // namespace AsyncInference

template <>
struct enable_bitmask_operators<AsyncInference::SocketStatus> {
    static constexpr bool enable = true;
};