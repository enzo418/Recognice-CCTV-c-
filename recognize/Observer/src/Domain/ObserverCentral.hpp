#pragma once

// include logging utils
#include "../Log/log.hpp"

// use SPD logger implementation
#include <filesystem>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "../IFunctionality.hpp"
#include "CameraObserver.hpp"
#include "CamerasFramesBlender.hpp"
#include "Configuration/Configuration.hpp"
#include "EventValidator.hpp"
#include "FrameDisplay.hpp"
#include "Notification/Notification.hpp"
#include "NotificationsController.hpp"

namespace Observer {
    template <typename TFrame>
    class ObserverCentral : public IFunctionality {
       public:
        explicit ObserverCentral(Configuration pConfig);

        void Start() override;

        void StopCamera(std::string id);
        void StopAllCameras();

        void StartCamera(std::string id);
        void StartAllCameras(bool useNotifications);

        void StartPreview();
        void StopPreview();

        void Stop() override;

        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

        void SubscribeToFrames(ISubscriber<TFrame>* sub);

       private:
        struct CameraThread {
            std::thread thread;
            std::shared_ptr<CameraObserver<TFrame>> camera;
            std::shared_ptr<EventValidator<TFrame>> eventValidator;
        };

        Configuration config;

        std::vector<CameraThread> camerasThreads;

        std::vector<std::pair<IFunctionality*, std::thread>>
            functionalityThreads;

        NotificationsController<TFrame> notificationController;

        FrameDisplay<TFrame> frameDisplay;

        CamerasFramesBlender<TFrame> framesBlender;

        CameraThread GetNewCameraThread(CameraConfiguration* cfg);

        void internalStopCamera(CameraThread& camThread);
        void internalStartCamera(CameraConfiguration* cfg);

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
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::Start() {
        bool useNotifications = this->config.localWebConfiguration.enabled ||
                                this->config.telegramConfiguration.enabled;

        OBSERVER_TRACE("Starting the cameras");
        this->StartAllCameras(useNotifications);

        OBSERVER_TRACE("Starting frames blender");
        this->functionalityThreads.emplace_back(
            &this->framesBlender,
            std::thread(&CamerasFramesBlender<TFrame>::Start,
                        &this->framesBlender));

        if (this->config.outputConfiguration.showOutput) {
            OBSERVER_TRACE("Starting the preview");
            this->StartPreview();
        }

        OBSERVER_TRACE("Starting the event validator");

        if (useNotifications) {
            OBSERVER_TRACE("Starting notification controller");
            this->functionalityThreads.emplace_back(
                &this->notificationController,
                std::thread(&NotificationsController<TFrame>::Start,
                            &this->notificationController));

            OBSERVER_TRACE("Subscribing notifications controller to events");
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::Stop() {
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

    template <typename TFrame>
    void ObserverCentral<TFrame>::StopAllCameras() {
        for (auto&& camThread : this->camerasThreads) {
            this->internalStopCamera(camThread);
        }

        // NOTE: Check for possible memory leak here
        this->camerasThreads.clear();
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
        for (auto& configuration : this->config.camerasConfiguration) {
            this->internalStartCamera(&configuration);
        }

        for (auto&& camThread : this->camerasThreads) {
            if (this->config.outputConfiguration.showOutput) {
                camThread.camera->SubscribeToFramesUpdate(&framesBlender);
            }

            if (useNotifications) {
                camThread.camera->SubscribeToCameraEvents(
                    camThread.eventValidator.get());

                // subscribe notification controller to validated events
                camThread.eventValidator->SubscribeToEventValidationDone(
                    &this->notificationController);
            }
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StartPreview() {
        this->functionalityThreads.emplace_back(
            &this->frameDisplay,
            std::thread(&FrameDisplay<TFrame>::Start, &this->frameDisplay));

        this->framesBlender.SubscribeToFramesUpdate(&this->frameDisplay);
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StopPreview() {
        this->frameDisplay.Stop();
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        for (auto&& camThread : this->camerasThreads) {
            camThread.camera->SubscribeToThresholdUpdate(subscriber);
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::internalStopCamera(
        ObserverCentral<TFrame>::CameraThread& camThread) {
        camThread.camera->Stop();

        if (camThread.thread.joinable()) {
            camThread.thread.join();
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::internalStartCamera(
        CameraConfiguration* cfg) {
        this->camerasThreads.push_back(this->GetNewCameraThread(cfg));
    }

    template <typename TFrame>
    typename ObserverCentral<TFrame>::CameraThread
    ObserverCentral<TFrame>::GetNewCameraThread(CameraConfiguration* cfg) {
        ObserverCentral<TFrame>::CameraThread ct;
        ct.camera = std::make_shared<CameraObserver<TFrame>>(cfg);
        ct.thread = std::thread(&CameraObserver<TFrame>::Start, ct.camera);

        // event validator
        ct.eventValidator = std::make_shared<EventValidator<TFrame>>(cfg);

        // Start event validator
        // TODO: If user wants notifications:
        this->functionalityThreads.emplace_back(
            // IFunctionality
            ct.eventValidator.get(),

            // std::thread
            std::thread(&EventValidator<TFrame>::Start, ct.eventValidator));

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
}  // namespace Observer
