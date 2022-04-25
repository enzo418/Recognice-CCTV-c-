#include "ObserverCentral.hpp"

#include "Configuration/CameraConfiguration.hpp"

namespace Observer {

    ObserverCentral::ObserverCentral(Configuration pConfig)
        : config(std::move(pConfig)),
          notificationController(&this->config),
          framesBlender(&this->config.outputConfiguration) {
        this->ProcessConfiguration();

        this->framesBlender.SetNumberCameras(
            static_cast<int>(this->config.camerasConfiguration.size()));

        this->functionalityThreads.emplace_back(&this->framesBlender);
        this->functionalityThreads.emplace_back(&this->frameDisplay);
        this->functionalityThreads.emplace_back(&this->notificationController);
    }

    void ObserverCentral::InternalStart() {
        bool useNotifications = this->config.localWebConfiguration.enabled ||
                                this->config.telegramConfiguration.enabled;

        if (!this->cameras.empty()) {
            this->StopAllCameras();
            this->cameras.clear();
        }

        OBSERVER_TRACE("Creating the cameras");
        this->CreateCameras(this->config.camerasConfiguration);

        OBSERVER_TRACE("Starting the cameras");
        this->StartAllCameras(useNotifications);

        OBSERVER_TRACE("Starting frames blender");
        framesBlender.Start();

        if (this->config.outputConfiguration.showOutput) {
            OBSERVER_TRACE("Starting the preview");
            this->StartPreview();
        }

        if (useNotifications) {
            OBSERVER_TRACE("Starting notification controller");
            this->notificationController.Start();
        }
    }

    void ObserverCentral::PostStop() {
        this->StopAllCameras();

        for (auto& functionality : this->functionalityThreads) {
            functionality->Stop();
        }
    }

    void ObserverCentral::StopAllCameras() {
        for (auto&& camera : this->cameras) {
            this->internalStopCamera(camera);
        }
    }

    void ObserverCentral::StopCamera(std::string id) {
        // TODO:
    }

    void ObserverCentral::StartCamera(std::string id) {
        // TODO:
        // get camera config based on id
        // call this->internalStartCamera(camcfg);
    }

    void ObserverCentral::StartAllCameras(bool useNotifications) {
        for (Camera& camera : this->cameras) {
            this->internalStartCamera(camera);

            // if (this->config.outputConfiguration.showOutput) {
            camera.camera->SubscribeToFramesUpdate(&framesBlender);
            // }

            if (useNotifications) {
                camera.camera->SubscribeToCameraEvents(
                    camera.eventValidator.get());

                // subscribe notification controller to validated events
                camera.eventValidator->SubscribeToEventValidationDone(
                    &this->notificationController);
            }
        }
    }

    void ObserverCentral::StartPreview() {
        frameDisplay.Start();

        this->framesBlender.SubscribeToFramesUpdate(&this->frameDisplay);
    }

    void ObserverCentral::StopPreview() { this->frameDisplay.Stop(); }

    void ObserverCentral::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        for (auto&& camera : this->cameras) {
            camera.camera->SubscribeToThresholdUpdate(subscriber);
        }
    }

    void ObserverCentral::internalStopCamera(ObserverCentral::Camera& camera) {
        camera.camera->Stop();
        camera.eventValidator->Stop();
    }

    void ObserverCentral::internalStartCamera(Camera& camera) {
        camera.camera->Start();
        camera.eventValidator->Start();
    }

    void ObserverCentral::CreateCameras(
        std::vector<CameraConfiguration>& camsCfg) {
        for (auto& cfg : camsCfg) {
            this->cameras.push_back(this->GetNewCamera(cfg));
        }
    }

    typename ObserverCentral::Camera ObserverCentral::GetNewCamera(
        CameraConfiguration& cfg) {
        ObserverCentral::Camera ct;

        if (cfg.type == ECameraType::VIEW) {
            ct.camera = std::make_shared<PasiveCamera>(&cfg);
        } else {
            ct.camera = std::make_shared<ActiveCamera>(&cfg);
        }

        // event validator
        ct.eventValidator = std::make_shared<EventValidator>(&cfg);

        return ct;
    }

    void ObserverCentral::ProcessConfiguration() {
        // 1. Check if media folder exists
        if (!std::filesystem::exists(this->config.mediaFolderPath)) {
            std::filesystem::create_directories(this->config.mediaFolderPath);
        }

        // 2. remove disabled cameras
        auto& camsConfig = this->config.camerasConfiguration;
        auto size = camsConfig.size();
        for (int i = 0; i < size; i++) {
            if (camsConfig[i].type == ECameraType::DISABLED) {
                camsConfig.erase(camsConfig.begin() + i);
                i--;
                size--;
            }
        }

        // 3. fix missing preview order
        std::sort(camsConfig.begin(), camsConfig.end(),
                  CompareCameraConfigurationsByPreviewOrder);
        size = camsConfig.size();
        for (int expected = 0; expected < size; expected++) {
            if (camsConfig[expected].positionOnOutput != expected) {
                camsConfig[expected].positionOnOutput = expected;
            }
        }
    }

    void ObserverCentral::SubscribeToFrames(ISubscriber<Frame>* sub) {
        this->framesBlender.SubscribeToFramesUpdate(sub);
    }

    void ObserverCentral::SubscribeToNewNotifications(
        INotificationEventSubscriber* subscriber) {
        this->notificationController.SubscribeToNewNotifications(subscriber);
    }
}  // namespace Observer
