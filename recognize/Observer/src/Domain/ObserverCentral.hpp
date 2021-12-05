#pragma once

// include logging utils
#include "../Log/log.hpp"

// use SPD logger implementation
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "CameraObserver.hpp"
#include "Configuration/Configuration.hpp"
#include "EventValidator.hpp"
#include "FrameDisplay.hpp"
#include "../IFunctionality.hpp"
#include "Notification/Notification.hpp"
#include "NotificationsController.hpp"

namespace Observer {
    template <typename TFrame>
    class ObserverCentral {
       public:
        explicit ObserverCentral(Configuration pConfig);

        bool Start();

        void StopCamera(std::string id);
        void StopAllCameras();

        void StartCamera(std::string id);
        void StartAllCameras();

        void StartPreview();
        void StopPreview();

        void Stop();

        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

       private:
        struct CameraThread {
            std::thread thread;
            std::shared_ptr<CameraObserver<TFrame>> camera;
        };

        Configuration config;

        std::vector<CameraThread> camerasThreads;

        std::vector<std::pair<IFunctionality*, std::thread>>
            functionalityThreads;

        NotificationsController<TFrame> notificationController;

        FrameDisplay<TFrame> frameDisplay;

        CameraThread GetNewCameraThread(CameraConfiguration* cfg);

        EventValidator<TFrame> eventValidator;

        void internalStopCamera(CameraThread& camThread);
        void internalStartCamera(CameraConfiguration* cfg);

        /* Proxy allows us to give our subscribers not only the threshold,
         * but also the camera that updated the threshold.
         */
        class ProxyCameraEventPublisher : public ICameraEventSubscriber<TFrame> {
           public:
            explicit ProxyCameraEventPublisher() {}

            void subscribe(IThresholdEventSubscriber* subscriber) {
                this->cameraEventPublisher.subscribe(subscriber);
            }

            void update(CameraConfiguration* cam,
                        CameraEvent<TFrame> ev) override {
                // TODO: Validate
            }

           private:
            Publisher<CameraConfiguration*, double> cameraEventPublisher;
        };
    };

    template <typename TFrame>
    ObserverCentral<TFrame>::ObserverCentral(Configuration pConfig)
        : frameDisplay(
              static_cast<int>(this->config.camerasConfiguration.size())),
          config(std::move(pConfig)),
          notificationController(&this->config) {}

    template <typename TFrame>
    bool ObserverCentral<TFrame>::Start() {
        OBSERVER_TRACE("Starting the cameras");
        this->StartAllCameras();

        if (this->config.outputConfiguration.showOutput) {
            OBSERVER_TRACE("Starting the preview");
            this->StartPreview();
        }

        OBSERVER_TRACE("Starting the event validator");
        // Start event validator
        // TODO: If user wants notifications:
        this->functionalityThreads.emplace_back(
            // IFunctionality
            &this->eventValidator,

            // std::thread
            std::thread(&EventValidator<TFrame>::Start, &this->eventValidator));

        OBSERVER_TRACE("Subscribing notifications controller to events");
        // subscribe notification controller to validated events
        this->eventValidator.SubscribeToEventValidationDone(
            &this->notificationController);

        return true;
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
    void ObserverCentral<TFrame>::StartAllCameras() {
        for (auto& configuration : this->config.camerasConfiguration) {
            this->internalStartCamera(&configuration);
        }

        for (auto&& camThread : this->camerasThreads) {
            camThread.camera->SubscribeToCameraEvents(&eventValidator);
            camThread.camera->SubscribeToFramesUpdate(&frameDisplay);
        }
    }

    template <typename TFrame>
    void ObserverCentral<TFrame>::StartPreview() {
        this->functionalityThreads.emplace_back(
            &this->frameDisplay,
            std::thread(&FrameDisplay<TFrame>::Start, &this->frameDisplay));
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
        return ct;
    }
}  // namespace Observer
