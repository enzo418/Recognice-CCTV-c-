#pragma once

#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "CameraObserver.hpp"
#include "Configuration.hpp"
#include "EventValidator.hpp"
#include "FrameDisplay.hpp"
#include "IFunctionality.hpp"
#include "Notification.hpp"
#include "NotificationsController.hpp"

namespace Observer {
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

        void SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber);

       private:
        struct CameraThread {
            std::thread thread;
            std::shared_ptr<CameraObserver> camera;
        };

        Configuration config;

        std::vector<CameraThread> camerasThreads;

        std::vector<std::pair<IFunctionality*, std::thread>>
            functionalityThreads;

        NotificationsController notificationController;

        FrameDisplay frameDisplay;

        CameraThread GetNewCameraThread(CameraConfiguration cfg);

        EventValidator eventValidator;

        void internalStopCamera(CameraThread& camThread);
        void internalStartCamera(CameraConfiguration cfg);

        /* Proxy allows us to give our subscribers not only the threshold,
         * but also the camera that updated the threshold.
         */
        class ProxyCameraEventPublisher : public CameraEventSubscriber {
           public:
            explicit ProxyCameraEventPublisher() {}

            void subscribe(ThresholdEventSubscriber* subscriber) {
                this->cameraEventPublisher.subscribe(subscriber);
            }

            void update(CameraConfiguration* cam, RawCameraEvent ev) override {
                // TODO: Validate
            }

           private:
            Publisher<CameraConfiguration*, double> cameraEventPublisher;
        };
    };

}  // namespace Observer
