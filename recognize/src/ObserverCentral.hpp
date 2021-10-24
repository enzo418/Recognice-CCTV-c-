#pragma once
#include "Configuration.hpp"
#include "Notification.hpp"
#include "CameraObserver.hpp"
#include "NotificationsController.hpp"
#include "FrameDisplay.hpp"

#include <opencv2/opencv.hpp>
#include <vector>
#include <thread>
#include <memory>

namespace Observer
{
    class ObserverCentral
    {
        public:
            ObserverCentral(Configuration pConfig);

            bool Start();

            void StopCamera(std::string id);
            void StopAllCameras();

            void StartCamera(std::string id);
            void StartAllCameras();

            void StartPreview();
            void StopPreview();

            void SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber);

        private:
            struct CameraThread {
                std::thread thread;
                std::shared_ptr<CameraObserver> camera;
            };

            Configuration config;

            std::vector<CameraThread> camerasThreads;

            NotificationsController notificationController;

            FrameDisplay frameDisplay;

            CameraThread GetNewCameraThread(CameraConfiguration cfg);

            void interalStopCamera(CameraThread& camThread);
            void interalStartCamera(CameraConfiguration cfg);
    };

} // namespace Observer
