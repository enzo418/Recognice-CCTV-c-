#pragma once

#include <algorithm>
#include <unordered_map>

#include "BaseCameraEvent.hpp"
#include "BaseObserverPattern.hpp"
#include "Configuration.hpp"
#include "Event.hpp"
#include "IFunctionality.hpp"
#include "MessagingService.hpp"
#include "RawCameraEvent.hpp"
#include "RestClientLocalWebNotifications.hpp"
#include "Semaphore.hpp"
#include "SimpleBlockingQueue.hpp"
#include "TelegramNotifications.hpp"
#include "utils/SpecialEnums.hpp"

namespace Observer {
    /**
     * @brief Send notifications asynchronously.
     * To push a notification use AddNotification, do not use
     * Send unless you do not mind thread locking.
     */
    class NotificationsController : public ISubscriber<Event, RawCameraEvent>,
                                    public IFunctionality {
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

        void update(Event event, RawCameraEvent rawCameraEvent) override;

        void Start() override;

        void Stop() override;

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
        bool running;

        void AddServiceToDrawable(MessagingService* service,
                                  NotificationsServiceConfiguration* cfg);

        std::vector<MessagingService*> services;

        std::unordered_map<int, std::vector<MessagingService*>>
            drawableServices;
        std::unordered_map<int, std::vector<MessagingService*>>
            notDrawableServices;

        Configuration* config;

        Semaphore smpQueue;

        SimpleBlockingQueue<TextNotification> textQueue;
        SimpleBlockingQueue<ImageNotification> imageQueue;
        SimpleBlockingQueue<VideoNotification> videoQueue;
        int groupID;
    };

}  // namespace Observer
