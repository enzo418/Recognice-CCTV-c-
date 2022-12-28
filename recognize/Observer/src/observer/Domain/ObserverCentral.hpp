#pragma once

// include logging utils
#include "observer/Log/log.hpp"

// use SPD logger implementation
#include <filesystem>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "ActiveCamera.hpp"
#include "CameraObserver.hpp"
#include "CamerasFramesBlender.hpp"
#include "Configuration/CameraConfiguration.hpp"
#include "Configuration/Configuration.hpp"
#include "EventValidator.hpp"
#include "FrameDisplay.hpp"
#include "Notification/Notification.hpp"
#include "NotificationsController.hpp"
#include "PasiveCamera.hpp"
#include "observer/Functionality.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    class ObserverCentral : public Functionality {
       public:
        explicit ObserverCentral(Configuration pConfig, int initialGroupID);

        void StopCamera(std::string id);
        void StopAllCameras();

        void StartCamera(std::string id);
        void StartAllCameras(bool useNotifications);

        void StartPreview();
        void StopPreview();

        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

        void SubscribeToFrames(ISubscriber<Frame>* sub);

        void SubscribeToNewNotifications(
            INotificationEventSubscriber* subscriber);

        /**
         * @brief If a camera event is triggered and event validator says it's
         * valid, then `subscriber` will be invoked.
         *
         * Note: NotificationController uses this publisher with LOW priority.
         *
         * @param subscriber
         */
        void SubscribeToValidCameraEvents(IEventValidatorSubscriber* subscriber,
                                          Priority priority);

       protected:
        void InternalStart() override;
        void PostStop() override;

       private:
        struct Camera {
            std::shared_ptr<CameraObserver> camera;
            std::shared_ptr<EventValidator> eventValidator;
        };

        Configuration config;

        std::vector<Camera> cameras;

        std::vector<IFunctionality*> functionalityThreads;

        NotificationsController notificationController;

        FrameDisplay frameDisplay;

        CamerasFramesBlender framesBlender;

        SynchronizedIDProvider groupIDProvider;

        void CreateCameras(std::vector<CameraConfiguration>& camsCfg);

        Camera GetNewCamera(CameraConfiguration& cfg);

        void internalStopCamera(Camera& camera);
        void internalStartCamera(Camera& cfg);

        void ProcessConfiguration();
    };

}  // namespace Observer
