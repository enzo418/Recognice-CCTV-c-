#pragma once

#include <functional>
#include <string_view>
#include <thread>

#include "grpcpp/grpcpp.h"
#include "inference.grpc.pb.h"
#include "observer/IFrame.hpp"
#include "observer/Semaphore.hpp"
#include "opencv2/opencv.hpp"
#include "types.hpp"

namespace AsyncInference {
    class DetectCaller;

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
         * received from the server
         * @param sendStrategy strategy that is used to send images to the
         * server. If null, all the images will be sent.
         * @return std::vector<ImageDetections> results received from the server
         */
        std::vector<ImageDetections> Detect(
            std::vector<Observer::Frame>& images,
            std::function<void(ImageDetections&)>&& onResult,
            ISendStrategy* sendStrategy = nullptr);

        /**
         * @brief Stops whatever the client is doing.
         * This function is called automatically when the client is destroyed.
         * Leaves the client in a state where IT CAN be used again.
         * It's safe to call this function multiple times.
         */
        void Stop();

       protected:
        typedef grpc::ClientAsyncReaderWriter<inference::Image,
                                              inference::DetectionResponse>
            WriterReader;
        void ReadResponses(
            WriterReader* rpc, std::vector<ImageDetections>& results,
            std::function<void(const ImageDetections&)>&& onResult);

       private:
        std::unique_ptr<inference::InferenceService::Stub> stub;
        std::shared_ptr<grpc::Channel> channel;

        std::unique_ptr<DetectCaller> caller;

        std::thread readerThread;
        std::atomic_bool stopFlag {false};
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