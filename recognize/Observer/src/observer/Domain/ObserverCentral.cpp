#include "ObserverCentral.hpp"
#include <algorithm>

#include "Configuration/CameraConfiguration.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    struct {
        bool operator()(const CameraConfiguration& struct1,
                        const CameraConfiguration& struct2) {
            return (struct1.positionOnOutput < struct2.positionOnOutput);
        }
    } CompareCameraConfigurationsByPreviewOrder;

    ObserverCentral::ObserverCentral(Configuration pConfig, int initialGroupID)
        : config(std::move(pConfig)),
          notificationController(&this->config),
          framesBlender(&this->config.outputConfiguration),
          groupIDProvider(initialGroupID) {
        this->ProcessConfiguration();

        if (this->config.cameras.empty()) {
            throw std::runtime_error("No cameras configured");
        }

        this->framesBlender.SetNumberCameras(
            static_cast<int>(this->config.cameras.size()));

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
        this->CreateCameras(this->config.cameras);

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

        this->onStartFinished();

        this->TaskRunner();
    }

    void ObserverCentral::PostStop() {
        this->StopAllCameras();

        for (auto& functionality : this->functionalityThreads) {
            functionality->Stop();
        }
    }

    /* ---------------------------------------------------------------- */
    /*                             CAMERAS                              */
    /* ---------------------------------------------------------------- */

    void ObserverCentral::StopAllCameras() {
        for (auto&& camera : this->cameras) {
            this->internalStopCamera(camera);
        }
    }

    bool ObserverCentral::StopCamera(const std::string& name) {
        for(auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                if (camera.camera->IsRunning()){
                    this->internalStopCamera(camera);
                }
                
                return true;
            }
        }
        
        OBSERVER_WARN("Couldn't stop '{}', not found.", name);
        
        return false;
    }

    bool ObserverCentral::StartCamera(const std::string& name) {
        for(auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                if (!camera.camera->IsRunning()){
                    this->internalStartCamera(camera);
                }
                
                return true;
            }
        }

        OBSERVER_WARN("Couldn't start '{}', not found.", name);

        return false;
    }

    bool ObserverCentral::SnoozeCamera(const std::string& name, int seconds) {
        for(auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                if (camera.camera->IsRunning()){
                    camera.snooze.timer.Start();
                    camera.snooze.seconds = seconds;
                    this->internalStopCamera(camera);
                }
                
                return true;
            }
        }


        OBSERVER_WARN("Couldn't snooze '{}', not found.", name);
        
        return false;
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
                // use low priority so hopefully is the last one. That's
                // important because it takes ownership of all the frames from
                // event.
                camera.eventValidator->SubscribeToValidEvent(
                    &this->notificationController, Priority::LOW);
            }
        }
    }

    void ObserverCentral::StartPreview() {
        frameDisplay.Start();

        this->framesBlender.SubscribeToFramesUpdate(&this->frameDisplay);
    }

    void ObserverCentral::StopPreview() { this->frameDisplay.Stop(); }

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
            ct.camera = std::make_shared<PassiveCamera>(&cfg);
        } else {
            ct.camera = std::make_shared<ActiveCamera>(&cfg);
        }

        // event validator
        ct.eventValidator =
            std::make_shared<EventValidator>(&cfg, &this->groupIDProvider);

        return ct;
    }

    void ObserverCentral::ProcessConfiguration() {
        // 1. Check if media folder exists
        if (!std::filesystem::exists(this->config.mediaFolderPath)) {
            std::filesystem::create_directories(this->config.mediaFolderPath);
        }

        // 2. remove disabled cameras
        auto& camsConfig = this->config.cameras;
        int size = camsConfig.size();
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

    /* ---------------------------------------------------------------- */
    /*                              EVENTS                              */
    /* ---------------------------------------------------------------- */

    void ObserverCentral::SubscribeToFrames(ISubscriber<Frame>* sub) {
        this->framesBlender.SubscribeToFramesUpdate(sub);
    }

    void ObserverCentral::SubscribeToNewNotifications(
        INotificationEventSubscriber* subscriber) {
        this->notificationController.SubscribeToNewNotifications(subscriber);
    }

    void ObserverCentral::SubscribeToValidCameraEvents(
        IEventValidatorSubscriber* subscriber, Priority priority) {
        for (auto& camera : this->cameras) {
            camera.eventValidator->SubscribeToValidEvent(subscriber, priority);
        }
    }

    void ObserverCentral::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        for (auto&& camera : this->cameras) {
            camera.camera->SubscribeToThresholdUpdate(subscriber);
        }
    }

    void ObserverCentral::OnStartFinished(std::function<void()>&& F) {
        this->onStartFinished = std::move(F);
    }

    /* ---------------------------------------------------------------- */
    /*                               TASKS                              */
    /* ---------------------------------------------------------------- */

    void ObserverCentral::TaskRunner() {
        while (this->IsRunning()) {
            this->TaskCheckCameraSnooze();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

    void ObserverCentral::TaskCheckCameraSnooze() {
        for (auto&& camera : this->cameras) {
            if (camera.snooze.timer.GetDuration() >
                camera.snooze.seconds) {
                this->internalStartCamera(camera);
            }
        }
    }

}  // namespace Observer
