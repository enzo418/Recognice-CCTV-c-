#pragma once

// include logging utils
#include "../Log/log.hpp"

// use SPD logger implementation
#include <filesystem>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "../Functionality.hpp"
#include "CameraObserver.hpp"
#include "CamerasFramesBlender.hpp"
#include "Configuration/CameraConfiguration.hpp"
#include "Configuration/Configuration.hpp"
#include "EventValidator.hpp"
#include "FrameDisplay.hpp"
#include "Notification/Notification.hpp"
#include "NotificationsController.hpp"

namespace Observer {
    template <typename TFrame>
    class ObserverCentral : public Functionality {
       public:
        explicit ObserverCentral(Configuration pConfig);

        void StopCamera(std::string id);
        void StopAllCameras();

        void StartCamera(std::string id);
        void StartAllCameras(bool useNotifications);

        void StartPreview();
        void StopPreview();

        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

        void SubscribeToFrames(ISubscriber<TFrame>* sub);

        void SubscribeToNewNotifications(
            INotificationEventSubscriber* subscriber);

       protected:
        void InternalStart() override;
        void PostStop() override;

       private:
        struct Camera {
            std::shared_ptr<CameraObserver<TFrame>> camera;
            std::shared_ptr<EventValidator<TFrame>> eventValidator;
        };

        Configuration config;

        std::vector<Camera> cameras;

        std::vector<IFunctionality*> functionalityThreads;

        NotificationsController<TFrame> notificationController;

        FrameDisplay<TFrame> frameDisplay;

        CamerasFramesBlender<TFrame> framesBlender;

        void CreateCameras(std::vector<CameraConfiguration>& camsCfg);

        Camera GetNewCamera(CameraConfiguration& cfg);

        void internalStopCamera(Camera& camera);
        void internalStartCamera(Camera& cfg);

        void ProcessConfiguration();
    };

    template <typename TFrame>
    ObserverCentral<TFrame>::ObserverCentral(Configuration pConfig)
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

    template <typename TFrame>
    void ObserverCentral<TFrame>::InternalStart() {
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

    template <typename TFrame>
    void ObserverCentral<TFrame>::PostStop() {
        this->StopAllCameras();

        for (auto& functionality : this->functionalityThreads) {
            functionality->Stop();
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StopAllCameras() {
        for (auto&& camera : this->cameras) {
            this->internalStopCamera(camera);
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StopCamera(std::string id) {
        // TODO:
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StartCamera(std::string id) {
        // TODO:
        // get camera config based on id
        // call this->internalStartCamera(camcfg);
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StartAllCameras(bool useNotifications) {
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

    template <typename TFrame>
    void ObserverCentral<TFrame>::StartPreview() {
        frameDisplay.Start();

        this->framesBlender.SubscribeToFramesUpdate(&this->frameDisplay);
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StopPreview() {
        this->frameDisplay.Stop();
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        for (auto&& camera : this->cameras) {
            camera.camera->SubscribeToThresholdUpdate(subscriber);
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::internalStopCamera(
        ObserverCentral<TFrame>::Camera& camera) {
        camera.camera->Stop();
        camera.eventValidator->Stop();
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::internalStartCamera(Camera& camera) {
        camera.camera->Start();
        camera.eventValidator->Start();
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::CreateCameras(
        std::vector<CameraConfiguration>& camsCfg) {
        for (auto& cfg : camsCfg) {
            this->cameras.push_back(this->GetNewCamera(cfg));
        }
    }

    template <typename TFrame>
    typename ObserverCentral<TFrame>::Camera
    ObserverCentral<TFrame>::GetNewCamera(CameraConfiguration& cfg) {
        ObserverCentral<TFrame>::Camera ct;
        ct.camera = std::make_shared<CameraObserver<TFrame>>(&cfg);

        // event validator
        ct.eventValidator = std::make_shared<EventValidator<TFrame>>(&cfg);

        return ct;
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::ProcessConfiguration() {
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

    template <typename TFrame>
    void ObserverCentral<TFrame>::SubscribeToFrames(ISubscriber<TFrame>* sub) {
        this->framesBlender.SubscribeToFramesUpdate(sub);
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::SubscribeToNewNotifications(
        INotificationEventSubscriber* subscriber) {
        this->notificationController.SubscribeToNewNotifications(subscriber);
    }
}  // namespace Observer
