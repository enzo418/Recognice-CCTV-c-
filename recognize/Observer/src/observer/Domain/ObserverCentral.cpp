#include "ObserverCentral.hpp"

#include <algorithm>

#include "Configuration/CameraConfiguration.hpp"
#include "observer/Domain/CameraObserver.hpp"
#include "observer/Log/log.hpp"
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

        // 4. fix videoValidatorBufferSize
        for (auto& camera : camsConfig) {
            if (camera.videoValidatorBufferSize <= 0) {
                camera.videoValidatorBufferSize = 5;
            }
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
        for (auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                if (camera.camera->IsRunning()) {
                    this->internalStopCamera(camera);
                }

                return true;
            }
        }

        OBSERVER_WARN("Couldn't stop '{}', not found.", name);

        return false;
    }

    bool ObserverCentral::StartCamera(const std::string& name) {
        for (auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                if (!camera.camera->IsRunning()) {
                    this->internalStartCamera(camera);

                    // reset to original type
                    if (camera.dynamicType.isActive()) {
                        OBSERVER_ASSERT(camera.dynamicType.originalType != -1,
                                        "Original type not set");

                        camera.camera->SetType(static_cast<ECameraType>(
                            camera.dynamicType.originalType));

                        camera.dynamicType.reset();
                    }
                }

                return true;
            }
        }

        OBSERVER_WARN("Couldn't start '{}', not found.", name);

        return false;
    }

    bool ObserverCentral::TemporarilyChangeCameraType(const std::string& name,
                                                      int seconds,
                                                      ECameraType type) {
        for (auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                camera.dynamicType.timer.Start();
                camera.dynamicType.seconds = seconds;
                camera.dynamicType.isIndefinitely = false;

                // save the original type if it wasn't saved yet
                if (camera.dynamicType.originalType == -1) {
                    camera.dynamicType.originalType = camera.camera->GetType();
                }

                // stop camera and event validator
                if (type == ECameraType::DISABLED) {
                    this->internalStopCamera(camera);
                }

                // let the camera know and handle the new type
                camera.camera->SetType(type);

                // start camera and event validator if needed
                if (!camera.camera->IsRunning() &&
                    type != ECameraType::DISABLED) {
                    this->internalStartCamera(camera);
                }

                return true;
            }
        }

        OBSERVER_WARN("Couldn't change type of '{}', not found.", name);

        return false;
    }

    bool ObserverCentral::IndefinitelyChangeCameraType(const std::string& name,
                                                       ECameraType type) {
        for (auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                const int originalType = camera.dynamicType.originalType;

                camera.dynamicType.reset();

                if (originalType == -1) {
                    // save the original type if it wasn't saved yet
                    camera.dynamicType.originalType = camera.camera->GetType();
                } else {
                    // else, keep the original type
                    camera.dynamicType.originalType = originalType;
                }

                camera.dynamicType.isIndefinitely = true;

                // stop camera and event validator, if needed
                if (type == ECameraType::DISABLED) {
                    this->internalStopCamera(camera);
                }

                // let the camera know and handle the new type
                camera.camera->SetType(type);

                // start camera and event validator, if needed
                if (!camera.camera->IsRunning() &&
                    type != ECameraType::DISABLED) {
                    this->internalStartCamera(camera);
                }

                return true;
            }
        }

        OBSERVER_WARN("Couldn't change type of '{}', not found.", name);

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

                // subscribe notification controller to validated
                // events use low priority so hopefully is the last
                // one. That's important because it takes ownership
                // of all the frames from event.
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

        ct.camera = std::make_shared<CameraObserver>(&cfg);

        // event validator
        ct.eventValidator =
            std::make_shared<EventValidator>(&cfg, &this->groupIDProvider);

        return ct;
    }

    CameraStatus ObserverCentral::GetCameraStatus(Camera& camera) {
        return CameraStatus {
            .name = camera.camera->GetName(),
            .currentType = camera.camera->GetType(),
            .dynamicType = {
                .active = camera.dynamicType.isActive(),
                .isIndefinitely = camera.dynamicType.isIndefinitely,
                .secondsLeft =
                    (int)::ceil(camera.dynamicType.seconds -
                                camera.dynamicType.timer.GetDuration()),
                .originalType = camera.dynamicType.isActive()
                                    ? static_cast<ECameraType>(
                                          camera.dynamicType.originalType)
                                    : camera.camera->GetType()}};
    }

    CameraStatus ObserverCentral::GetCameraStatus(const std::string& name) {
        for (auto&& camera : this->cameras) {
            if (camera.camera->GetName() == name) {
                return GetCameraStatus(camera);
            }
        }

        OBSERVER_WARN("Couldn't get status of '{}', not found.", name);

        return CameraStatus {};
    }

    std::vector<CameraStatus> ObserverCentral::GetCamerasStatus() {
        std::vector<CameraStatus> camerasStatus;

        for (auto&& camera : this->cameras) {
            camerasStatus.push_back(GetCameraStatus(camera));
        }

        return camerasStatus;
    }

    /* ------------------------------------------------------ */
    /*                         EVENTS                         */
    /* ------------------------------------------------------ */

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

    /* ----------------------------------------------------------------
     */
    /*                               TASKS */
    /* ----------------------------------------------------------------
     */

    void ObserverCentral::TaskRunner() {
        while (running) {
            this->TaskCheckCameraSnooze();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

    void ObserverCentral::TaskCheckCameraSnooze() {
        for (auto&& camera : this->cameras) {
            if (camera.dynamicType.isActive() &&
                !camera.dynamicType.isIndefinitely &&
                camera.dynamicType.timer.GetDuration() >
                    camera.dynamicType.seconds) {
                ECameraType currentType = camera.camera->GetType();

                // restart type
                camera.camera->SetType(
                    static_cast<ECameraType>(camera.dynamicType.originalType));

                // start camera and event validator if needed
                if (currentType == ECameraType::DISABLED) {
                    this->internalStartCamera(camera);
                }

                // reset snooze
                camera.dynamicType.reset();
            }
        }
    }

}  // namespace Observer
