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
#include "observer/Functionality.hpp"
#include "observer/Pattern/ObserverBasics.hpp"

namespace Observer {
    struct CameraStatus {
        struct DynamicType {
            bool active {false};
            bool isIndefinitely {false};
            int secondsLeft {0};
            ECameraType originalType;
        };

        std::string name;
        ECameraType currentType;

        DynamicType dynamicType;
    };

    class ObserverCentral : public Functionality {
       public:
        explicit ObserverCentral(Configuration pConfig, int initialGroupID);

        /* ---------------------------- CAMERAS --------------------------- */
        bool StopCamera(const std::string& name);
        void StopAllCameras();

        /**
         * @brief Temporarily change the camera type, then after `seconds` it
         * will be changed back to the original type.
         *
         * @param name camera name
         * @param seconds how long to change the camera type
         * @param type new camera type
         * @return true if the camera was found and the type was changed
         */
        bool TemporarilyChangeCameraType(
            const std::string& name, int seconds,
            ECameraType type = ECameraType::DISABLED);

        /**
         * @brief Change the camera type indefinitely.
         *
         * @param name camera name
         * @param type new camera type
         * @return true if the camera was found and the type was changed
         */
        bool IndefinitelyChangeCameraType(const std::string& name,
                                          ECameraType type);

        bool StartCamera(const std::string& name);
        void StartAllCameras(bool useNotifications);

        CameraStatus GetCameraStatus(const std::string& name);
        std::vector<CameraStatus> GetCamerasStatus();

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
                // the camera type will be indefinitely changed to this type
                bool isIndefinitely {false};

                // timer for the temporal type, set if isIndefinitely == false
                Timer<std::chrono::seconds> timer;

                // temp type remaining seconds, set if isIndefinitely == false
                unsigned int seconds {0};

                // camera type before the dynamic type was set
                int originalType {-1};

                bool isActive() const { return seconds > 0 || isIndefinitely; }

                void reset() {
                    timer.Stop();
                    seconds = 0;
                    originalType = -1;
                    isIndefinitely = false;
                }
            } dynamicType;
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

       private:
        CameraStatus GetCameraStatus(Camera& camera);
    };

}  // namespace Observer
