#include "ObserverCentral.hpp"

#include <utility>

namespace Observer {
    ObserverCentral::ObserverCentral(Configuration pConfig)
        : frameDisplay(
              static_cast<int>(this->config.camerasConfiguration.size())),
          config(std::move(pConfig)),
          notificationController(&this->config) {}

    bool ObserverCentral::Start() {
        this->StartAllCameras();

        if (this->config.outputConfiguration.showOutput) {
            this->StartPreview();
        }

        // Start event validator
        // TODO: If user wants notifications:
        this->functionalityThreads.emplace_back(
            // IFunctionality
            &this->eventValidator,

            // std::thread
            std::thread(&EventValidator::Start, &this->eventValidator));

        // subscribe notification controller to validated events
        this->eventValidator.SubscribeToEventValidationDone(
            &this->notificationController);

        return true;
    }

    void ObserverCentral::Stop() {
        this->StopAllCameras();

        for (auto& funcThread : this->functionalityThreads) {
            std::get<0>(funcThread)->Stop();
            auto& thread = std::get<1>(funcThread);
            if (thread.joinable()) {
                thread.join();
            }
        }

        this->functionalityThreads.clear();
    }

    void ObserverCentral::StopAllCameras() {
        for (auto&& camThread : this->camerasThreads) {
            this->internalStopCamera(camThread);
        }

        // NOTE: Check for possible memory leak here
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
        for (auto&& configuration : this->config.camerasConfiguration) {
            this->internalStartCamera(configuration);
        }

        for (auto&& camThread : this->camerasThreads) {
            camThread.camera->SubscribeToCameraEvents(&eventValidator);
            camThread.camera->SubscribeToFramesUpdate(&frameDisplay);
        }
    }

    void ObserverCentral::StartPreview() {
        this->functionalityThreads.emplace_back(
            &this->frameDisplay,
            std::thread(&FrameDisplay::Start, &this->frameDisplay));
    }

    void ObserverCentral::StopPreview() { this->frameDisplay.Stop(); }

    void ObserverCentral::SubscribeToThresholdUpdate(
        ThresholdEventSubscriber* subscriber) {
        for (auto&& camThread : this->camerasThreads) {
            camThread.camera->SubscribeToThresholdUpdate(subscriber);
        }
    }

    void ObserverCentral::internalStopCamera(
        ObserverCentral::CameraThread& camThread) {
        camThread.camera->Stop();

        if (camThread.thread.joinable()) {
            camThread.thread.join();
        }
    }

    void ObserverCentral::internalStartCamera(CameraConfiguration cfg) {
        this->camerasThreads.push_back(this->GetNewCameraThread(cfg));
    }

    ObserverCentral::CameraThread ObserverCentral::GetNewCameraThread(
        CameraConfiguration cfg) {
        ObserverCentral::CameraThread ct;
        ct.camera = std::make_shared<CameraObserver>(&cfg);
        ct.thread = std::thread(&CameraObserver::Start, ct.camera);
        return ct;
    }
}  // namespace Observer
