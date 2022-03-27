#pragma once

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include "../Functionality.hpp"
#include "../IFrame.hpp"
#include "../Pattern/Camera/ICameraEventSubscriber.hpp"
#include "../Pattern/Event/IEventSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Semaphore.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "../Utils/NotificationTypesHelpers.hpp"
#include "../Utils/SpecialEnums.hpp"
#include "Configuration/Configuration.hpp"
#include "Configuration/NotificationsServiceConfiguration.hpp"
#include "Event/CameraEvent.hpp"
#include "Event/Event.hpp"
#include "Notification/DTONotification.hpp"
#include "Notification/IMessagingService.hpp"
#include "Notification/LocalNotifications.hpp"
#include "Notification/TelegramNotifications.hpp"

namespace Observer {
    /**
     * @brief Send notifications asynchronously.
     * To push a notification use AddNotification, do not use
     * Send unless you do not mind thread locking.
     */
    class NotificationsController : public IEventSubscriber,
                                    public Functionality {
       public:
        explicit NotificationsController(Configuration* cfg);
        ~NotificationsController();

        /**
         * @brief Adds a notification to the notifications queue
         *
         * @param notification
         */
        void AddNotification(TextNotification textNotf);

        /**
         * @brief Adds a notification to the notifications queue
         *
         * @param notification
         */
        void AddNotification(ImageNotification imageNotf);

        /**
         * @brief Adds a notification to the notifications queue
         *
         * @param notification
         */
        void AddNotification(VideoNotification videoNotf);

        void update(Event event, CameraEvent rawCameraEvent) override;

        void SubscribeToNewNotifications(
            INotificationEventSubscriber* subscriber);

        void InternalStart() override;

       protected:
        /**
         * @brief Main loop that consumes the notifications queue
         */
        void ConsumeNotifications();

        /**
         * @brief sends a text notifications to all services.
         * @param notification
         */
        void Send(TextNotification notification);

        /**
         * @brief build and sends a image notifications to all services.
         * @param notification
         */
        void Send(ImageNotification notification);

        /**
         * @brief build and sends a video notifications to all services.
         * @param notification
         */
        void Send(VideoNotification notification);

       private:
        void AddService(IMessagingService* service,
                        NotificationsServiceConfiguration* cfg);

        std::vector<IMessagingService*> services;

        std::map<std::tuple<IMessagingService*, int>, bool> servicesType;

        std::unordered_map<int, std::vector<IMessagingService*>>
            drawableServices;
        std::unordered_map<int, std::vector<IMessagingService*>>
            notDrawableServices;

        Configuration* config;

        Semaphore smpQueue;

        SimpleBlockingQueue<TextNotification> textQueue;
        SimpleBlockingQueue<ImageNotification> imageQueue;
        SimpleBlockingQueue<VideoNotification> videoQueue;

        // non-owning ptr
        LocalNotifications* localNotifications {nullptr};

        int groupID;
    };
}  // namespace Observer
