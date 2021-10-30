#pragma once

#include "Configuration.hpp"

#include "MessagingService.hpp"
#include "TelegramNotifications.hpp"
#include "LocalWebNotifications.hpp"

#include "Semaphore.hpp"

#include "SimpleBlockingQueue.hpp"

#include <unordered_map>
#include <algorithm>

#include "BaseObserverPattern.hpp"
#include "RawCameraEvent.hpp"

namespace Observer
{   
    class CameraEventSubscriber : public ISubscriber<CameraConfiguration*, RawCameraEvent> {
        virtual void update(CameraConfiguration* cam, RawCameraEvent ev) = 0;
    };

    /**
     * @brief Send notifications asynchronously.
     * To push a notification use AddNotification, do not use 
     * Send unless you do not mind thread locking.
     */
    class NotificationsController : public CameraEventSubscriber
    {
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

        void update(CameraConfiguration* cam, RawCameraEvent ev);

    protected:

        /**
         * @brief Main loop that consumes the notifications queue
         */
        void ConsumeNotifications();

    private:
        void AddServiceToDrawable(MessagingService* service, NotificationsServiceConfiguration* cfg);

        std::vector<MessagingService*> services;

        std::unordered_map<int, std::vector<MessagingService*>> drawableServices;
        std::unordered_map<int, std::vector<MessagingService*>> notDrawableServices;

        Configuration* config;

        Semaphore smpQueue;

        SimpleBlockingQueue<TextNotification> textQueue;
        SimpleBlockingQueue<ImageNotification> imageQueue;
        SimpleBlockingQueue<VideoNotification> videoQueue;
        int groupID;
    };

} // namespace Observer
