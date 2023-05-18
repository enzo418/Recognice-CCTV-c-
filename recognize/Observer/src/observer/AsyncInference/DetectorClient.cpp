#include "DetectorClient.hpp"

#include <grpcpp/client_context.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/channel_arguments.h>
#include <grpcpp/support/status.h>

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <vector>

#include "inference.grpc.pb.h"
#include "inference.pb.h"
#include "observer/Implementation.hpp"
#include "observer/Semaphore.hpp"
#include "types.hpp"

namespace AsyncInference {
    /* ------------------------------------------------------ */
    /*               SendEveryNthFrame Strategy               */
    /* ------------------------------------------------------ */
    SendEveryNthFrame::SendEveryNthFrame(int n) { this->n = n; }

    bool SendEveryNthFrame::ShouldSend(Observer::Frame&, int index) {
        return index % n == 0;
    }

    /* ------------------------------------------------------ */
    /*                      DetectCaller                      */
    /* ------------------------------------------------------ */
    class DetectCaller
        : public grpc::ClientBidiReactor<inference::Image,
                                         inference::DetectionResponse> {
       public:
        DetectCaller(inference::InferenceService::Stub* stub,
                     grpc::ClientContext* pContext,
                     std::vector<Observer::Frame>* pImages,
                     std::function<void(ImageDetections&)>&& pOnResult,
                     DetectorClient::ISendStrategy* pSendStrategy)
            : onResult(pOnResult),
              images(pImages),
              context(pContext),
              sendStrategy(pSendStrategy) {
            stub->async()->Detect(pContext, this);
            NextWrite();           // post first write for later
            StartRead(&response);  // post it for later
            StartCall();           // invokes pending write, read
        }

        // GRPC methods
       private:
        inference::DetectionResponse response;
        std::function<void(ImageDetections&)> onResult;
        std::atomic_bool stopFlag {false};
        std::vector<Observer::Frame>* images;
        grpc::ClientContext* context;
        size_t currentImage {0};
        Semaphore doneSmp;
        grpc::Status status;
        std::vector<ImageDetections> results;
        inference::Image request;
        std::vector<uchar> buffer;
        DetectorClient::ISendStrategy* sendStrategy;

       public:
        grpc::Status Await() {
            doneSmp.acquire();
            return status;
        }

        std::vector<ImageDetections>&& GetResults() {
            return std::move(results);
        }

        void Cancel() {
            if (stopFlag) {
                return;
            }

            stopFlag = true;
            context->TryCancel();
        }

       private:
        void OnWriteDone(bool ok) override {
            if (ok) {
                NextWrite();
            } else {
                std::cout
                    << "DetectorClient Write RPC finished (done or failed)."
                    << std::endl;
            }
        }

        void OnReadDone(bool ok) override {
            if (ok) {
                ImageDetections image_detections;
                image_detections.image_index = response.image_id();

                image_detections.detections.reserve(response.detections_size());

                for (int i = 0; i < response.detections_size(); i++) {
                    const inference::Detection& detection =
                        response.detections(i);
                    SingleDetection result;

                    const auto& box = detection.bounding_box();
                    result.x = box.x();
                    result.y = box.y();
                    result.width = box.width();
                    result.height = box.height();

                    result.confidence = detection.confidence();
                    result.label = detection.label();

                    image_detections.detections.push_back(result);
                }

                onResult(image_detections);
                results.push_back(image_detections);

                StartRead(&response);
            } else {
                std::cout
                    << "DetectorClient Read RPC finished (done or failed)."
                    << std::endl;
            }
        }

        void OnDone(const grpc::Status& s) override {
            std::cout << "DetectorClient OnDone. Cancelled? "
                      << (s.error_code() == grpc::StatusCode::CANCELLED)
                      << std::endl;
            doneSmp.release();
            status = s;
        }

        void NextWrite() {
            if (stopFlag) {
                std::cout << "DetectorClient stopped." << std::endl;
                return;
            }

            if (currentImage >= images->size()) {
                std::cout << "DetectorClient finished writing images."
                          << std::endl;
                this->StartWritesDone();
                return;
            }

            if (sendStrategy != nullptr &&
                !sendStrategy->ShouldSend(images->at(currentImage),
                                          currentImage)) {
                currentImage++;
                NextWrite();
                return;
            }

            auto& image = images->at(currentImage);

            if (!image.EncodeImage(".jpg", 95, buffer)) {
                std::cout << "DetectorClient failed to encode image."
                          << std::endl;
                return;
            }

            request.set_image_id(currentImage);
            auto imSz = image.GetSize();
            request.set_cols(imSz.width);
            request.set_rows(imSz.height);
            request.set_data(buffer.data(), buffer.size());

            this->StartWrite(&request);

            currentImage++;
        }
    };

    DetectorClient::DetectorClient(const std::string& server_address) {
        grpc::ChannelArguments args;
        // args.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
        channel = grpc::CreateCustomChannel(
            server_address, grpc::InsecureChannelCredentials(), args);

        stub = inference::InferenceService::NewStub(channel);
    }

    std::vector<std::string> DetectorClient::GetModelNames() {
        grpc::ClientContext context;

        inference::ModelList response;

        grpc::Status status =
            stub->GetModelNames(&context, inference::Empty(), &response);

        if (!status.ok()) {
            std::cout << "GetModelNames rpc failed." << std::endl;
            return {};
        }

        std::vector<std::string> modelNames;
        modelNames.reserve(response.model_names_size());

        for (int i = 0; i < response.model_names_size(); i++) {
            modelNames.push_back(response.model_names(i));
        }

        return modelNames;
    }

    std::vector<ImageDetections> DetectorClient::Detect(
        std::vector<Observer::Frame>& images,
        std::function<void(ImageDetections&)>&& onResult,
        ISendStrategy* sendStrategy) {
        grpc::ClientContext context;
        context.AddMetadata("model-type", "fast");

        caller = std::make_unique<DetectCaller>(
            stub.get(), &context, &images, std::move(onResult), sendStrategy);
        grpc::Status status = caller->Await();
        if (!status.ok()) {
            std::cout << "detect rpc failed." << std::endl;
        }

        std::vector<ImageDetections> results = caller->GetResults();

        caller.reset();

        return results;
    }

    void DetectorClient::Stop() {
        if (caller) {
            caller->Cancel();
        }
    }

    DetectorClient::~DetectorClient() { this->Stop(); }
}  // namespace AsyncInference