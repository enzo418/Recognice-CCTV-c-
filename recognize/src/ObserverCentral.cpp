#include "ObserverCentral.hpp"

namespace Observer
{
        ObserverCentral::ObserverCentral(Configuration pConfig)
        :   notificationController(&this->config),
            frameDisplay(this->config.camerasConfiguration.size())
        {
            this->config = pConfig;
        }

        bool ObserverCentral::Start() {
            this->StartAllCameras();

            if (this->config.outputConfiguration.showOutput) {
                this->StartPreview();
            }

            return true;
        }

        void ObserverCentral::StopAllCameras() {
            for (auto &&camThread : this->camerasThreads)
            {
                this->internalStopCamera(camThread);
            }
            
            // TODO: Check for possible memory leak here
            this->camerasThreads.clear();
        }

        void ObserverCentral::StopCamera(std::string id) {
            // TODO:
        }

        void ObserverCentral::StartCamera(std::string id) {
            // TODO:
            // get camera config based on id
            // call this->internalStartCamera(camcfg);
        }

        void ObserverCentral::StartAllCameras() {
            for (auto &&camcfg : this->config.camerasConfiguration)
            {
                this->internalStartCamera(camcfg);
            }

            for (auto &&camThread : this->camerasThreads)
            {
                camThread.camera->SubscribeToCameraEvents(&notificationController);
                camThread.camera->SubscribeToFramesUpdate(&frameDisplay);
            }
        }

        void ObserverCentral::StartPreview() {
            this->frameDisplay.Start();
        }

        void ObserverCentral::StopPreview() {
            this->frameDisplay.Stop();
        }

        void ObserverCentral::SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber) {
            for (auto &&camThread : this->camerasThreads)
            {
                camThread.camera->SubscribeToThresholdUpdate(subscriber);
            }
        }

        void ObserverCentral::internalStopCamera(ObserverCentral::CameraThread& camThread) {
            camThread.camera->Stop();

            if (camThread.thread.joinable()){
                camThread.thread.join();
            }
        }

        void ObserverCentral::internalStartCamera(CameraConfiguration cfg) {
            this->camerasThreads.push_back(this->GetNewCameraThread(cfg));
        }

        ObserverCentral::CameraThread ObserverCentral::GetNewCameraThread(CameraConfiguration cfg) {
            ObserverCentral::CameraThread ct;
            ct.camera = std::make_shared<CameraObserver>(&cfg);
            ct.thread = std::thread(&CameraObserver::Start, ct.camera);
            return ct;
        }
} // namespace Observer
