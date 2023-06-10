#pragma once

// include logging utils
#include "observer/Log/log.hpp"

// use SPD logger implementation
#include <chrono>
#include <filesystem>
#include <memory>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

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

        /* ---------------------------- CAMERAS --------------------------- */
        bool StopCamera(const std::string& name);
        void StopAllCameras();

        bool TemporarilyChangeCameraType(
            const std::string& name, int seconds,
            ECameraType type = ECameraType::DISABLED);

        bool StartCamera(const std::string& name);
        void StartAllCameras(bool useNotifications);

        /* ---------------------------- PREVIEW --------------------------- */
        void StartPreview();
        void StopPreview();

        /* ---------------------------- EVENTS ---------------------------- */
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

        /**
         * @brief Subscribe to be notified when all cameras, processors, etc.
         * are started.
         *
         * @param F callback
         */
        void OnStartFinished(std::function<void()>&& F);

       protected:
        void InternalStart() override;
        void PostStop() override;

       private:
        std::function<void()> onStartFinished;

       private:  // INTERNAL TASKS
        void TaskRunner();
        void TaskCheckCameraSnooze();

       private:
        struct Camera {
            std::shared_ptr<CameraObserver> camera;
            std::shared_ptr<EventValidator> eventValidator;

            struct {
                Timer<std::chrono::seconds> timer;
                int seconds;
            } snooze;
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
