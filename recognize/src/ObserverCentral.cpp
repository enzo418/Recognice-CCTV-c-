#include "ObserverCentral.hpp"

namespace Observer
{
        ObserverCentral::ObserverCentral() {

        }

        bool ObserverCentral::Start(Configuration pConfig) {
            this->config = pConfig;
            this->StartAllCameras();

            if (this->config.outputConfiguration.showOutput) {
                this->outputFrames.reserve(this->config.camerasConfiguration.size());
                this->StartPreview();
            }
        }

        void ObserverCentral::StopCamera(std::string id) {
            
        }

        void ObserverCentral::StopAllCameras() {
            for (auto &&camThread : this->camerasThreads)
            {
                this->interalStopCamera(camThread);
            }
            
            // TODO: Check for possible memory leak here
            this->camerasThreads.clear();
        }

        void ObserverCentral::StartCamera(std::string id) {
            // TODO:
            // get camera config based on id
            // call this->interalStartCamera(camcfg);
        }

        void ObserverCentral::StartAllCameras() {
            for (auto &&camcfg : this->config.camerasConfiguration)
            {
                this->interalStartCamera(camcfg);
            }

            for (auto &&camThread : this->camerasThreads)
            {
                camThread.camera->SubscribeToCameraEvents(&notificationController);
                camThread.camera->SubscribeToFramesUpdate(&frameDisplay);
            }
        }

        void ObserverCentral::StartPreview() {
            // TODO:
        }

        void ObserverCentral::StopPreview() {
            // TODO:
        }

        void ObserverCentral::SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber) {
            for (auto &&camThread : this->camerasThreads)
            {
                camThread.camera->SubscribeToThresholdUpdate(subscriber);
            }
        }

        void ObserverCentral::interalStopCamera(ObserverCentral::CameraThread& camThread) {
            camThread.camera->Stop();

            if (camThread.thread.joinable()){
                camThread.thread.join();
            }
        }

        void ObserverCentral::interalStartCamera(CameraConfiguration cfg) {
            this->camerasThreads.push_back(this->GetNewCameraThread(cfg));
        }

        ObserverCentral::CameraThread ObserverCentral::GetNewCameraThread(CameraConfiguration cfg) {
            ObserverCentral::CameraThread ct;
            ct.camera = std::make_shared<CameraObserver>(&cfg);
            ct.thread = std::thread(&CameraObserver::Start, ct.camera);
            return ct;
        }
} // namespace Observer
