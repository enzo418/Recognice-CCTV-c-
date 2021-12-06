#pragma once

#include <algorithm>
#include <unordered_map>

#include "../IFunctionality.hpp"
#include "../Pattern/Camera/ICameraEventSubscriber.hpp"
#include "../Pattern/Event/IEventSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Semaphore.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "../Utils/SpecialEnums.hpp"
#include "Configuration/Configuration.hpp"
#include "Event/CameraEvent.hpp"
#include "Event/Event.hpp"
#include "Notification/IMessagingService.hpp"
#include "Notification/RestClientLocalWebNotifications.hpp"
#include "Notification/TelegramNotifications.hpp"

namespace Observer {
    /**
     * @brief Send notifications asynchronously.
     * To push a notification use AddNotification, do not use
     * Send unless you do not mind thread locking.
     */
    template <typename TFrame>
    class NotificationsController : public IEventSubscriber<TFrame>,
                                    public IFunctionality {
        /**
         * @brief Todo: Make a specializations of this class as opencv,
         * NotificationsConntrollerOPV,
         * ImageNotificationOPV or OC or OpenCv,
         * VideoNotOV...
         */
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
        void AddNotification(ImageNotification<TFrame> imageNotf);

        /**
         * @brief Adds a notification to the notifications queue
         *
         * @param notification
         */
        void AddNotification(VideoNotification<TFrame> videoNotf);

        void update(Event event, CameraEvent<TFrame> rawCameraEvent) override;

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
        void Send(ImageNotification<TFrame> notification);

        /**
         * @brief build and sends a video notifications to all services.
         * @param notification
         */
        void Send(VideoNotification<TFrame> notification);

       private:
        bool running;

        void AddServiceToDrawable(IMessagingService* service,
                                  NotificationsServiceConfiguration* cfg);

        std::vector<IMessagingService*> services;

        std::unordered_map<int, std::vector<IMessagingService*>>
            drawableServices;
        std::unordered_map<int, std::vector<IMessagingService*>>
            notDrawableServices;

        Configuration* config;

        Semaphore smpQueue;

        SimpleBlockingQueue<TextNotification> textQueue;
        SimpleBlockingQueue<ImageNotification<TFrame>> imageQueue;
        SimpleBlockingQueue<VideoNotification<TFrame>> videoQueue;
        int groupID;
    };

    template <typename TFrame>
    NotificationsController<TFrame>::NotificationsController(
        Configuration* cfg) {
        this->config = cfg;
        this->groupID = 0;

        if (cfg->localWebConfiguration.enabled) {
            auto ptrLocal = new RestClientLocalWebNotifications(
                cfg->localWebConfiguration.webServerUrl);

            this->services.push_back(ptrLocal);

            this->AddServiceToDrawable(ptrLocal,
                                       &this->config->localWebConfiguration);
        } else if (cfg->telegramConfiguration.enabled) {
            auto ptrTelegram =
                new TelegramNotifications(&cfg->telegramConfiguration);

            this->services.push_back(ptrTelegram);

            this->AddServiceToDrawable(ptrTelegram,
                                       &this->config->telegramConfiguration);
        }

        this->running = false;
    }

    template <typename TFrame>
    NotificationsController<TFrame>::~NotificationsController() {
        // delete all the memory allocated here
        for (auto sp : this->services) {
            delete sp;
        }
        this->services.clear();
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Send(TextNotification notification) {
        // 1. For each service call Send(TextNotification)
        for (auto&& service : this->services) {
            service->SendText(notification.GetCaption());
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Send(
        ImageNotification<TFrame> notification) {
        // 1. Build
        notification.BuildNotification(this->config->mediaFolderPath);

        // 2. For each service that doesn't need the trace call
        // SendVideo(imagepath)
        const std::string& path = notification.GetImagePath();

        for (auto&& service :
             this->notDrawableServices[flag_to_int(ETrazable::IMAGE)]) {
            service->SendImage(path, notification.GetCaption());
        }

        auto& servD = this->drawableServices[flag_to_int(ETrazable::IMAGE)];
        // guard: if there is at leat 1 service that need the trace
        if (!servD.empty()) {
            // 3. Draw trace on image

            // 4. For each service that need the trace call
            // SendVideo(image2path)
            for (auto&& service : servD) {
            }
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Send(
        VideoNotification<TFrame> notification) {
        // 1. Build
        // notification.BuildNotification(this->config->mediaFolderPath, )

        // 2. For each service that doesn't need the trace call
        // SendVideo(videopath)
        for (auto&& service :
             this->notDrawableServices[flag_to_int(ETrazable::VIDEO)]) {
        }

        // guard: if there is at leat 1 service that need the trace
        if (!this->drawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 3. Draw trace on video

            // 4. For each service that need the trace call
            // SendVideo(video2path)
            for (auto&& service :
                 this->drawableServices[flag_to_int(ETrazable::VIDEO)]) {
            }
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::AddNotification(
        TextNotification textNotf) {
        this->textQueue.push(textNotf);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::AddNotification(
        ImageNotification<TFrame> imageNotf) {
        this->imageQueue.push(imageNotf);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::AddNotification(
        VideoNotification<TFrame> videoNotf) {
        this->videoQueue.push(videoNotf);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::ConsumeNotifications() {
        while (this->running) {
            // 1. semaphore adquire - There is at leat 1 notification
            smpQueue.acquire();

            /// TODO: Improve quality with a unorded_map with the queues, like
            /// drawable
            if (this->textQueue.size() > 0) {
                this->Send(std::move(this->textQueue.pop()));
            } else if (this->imageQueue.size() > 0) {
                this->Send(std::move(this->imageQueue.pop()));
            } else if (this->videoQueue.size() > 0) {
                this->Send(std::move(this->videoQueue.pop()));
            }
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Start() {
        this->running = true;
        this->ConsumeNotifications();
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::AddServiceToDrawable(
        IMessagingService* service, NotificationsServiceConfiguration* cfg) {
        if (has_flag(cfg->drawTraceOfChangeOn, ETrazable::IMAGE)) {
            drawableServices[flag_to_int(ETrazable::IMAGE)].push_back(service);
        } else {
            notDrawableServices[flag_to_int(ETrazable::IMAGE)].push_back(
                service);
        }

        if (has_flag(cfg->drawTraceOfChangeOn, ETrazable::VIDEO)) {
            drawableServices[flag_to_int(ETrazable::VIDEO)].push_back(service);
        } else {
            notDrawableServices[flag_to_int(ETrazable::VIDEO)].push_back(
                service);
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::update(
        Event event, CameraEvent<TFrame> rawCameraEvent) {
        // 1. Create a text notification
        // 1. a. Get camera name
        std::string cameraName = event.GetCameraName();

        // 1. b. Replace template text notification
        std::string text = SpecialFunctions::FormatNotificationTextString(
            this->config->notificationTextTemplate, cameraName);

        // 1. c. Create the notification
        TextNotification textNotification(this->groupID, event, text);

        // 2. Create an image notification using the first frame where the event
        // happen
        int indexFirst = event.GetFirstFrameWhereFindingWasFound();
        ImageNotification<TFrame> imageNotification(
            this->groupID, event, cameraName,
            rawCameraEvent.GetFrameAt(indexFirst));

        // 3. Create a video notification using the frames
        VideoNotification<TFrame> videoNotification(
            this->groupID, event, cameraName,
            std::move(rawCameraEvent.PopFrames()));

        // 4. call AddNotification for each one
        this->AddNotification(textNotification);
        this->AddNotification(imageNotification);
        this->AddNotification(videoNotification);

        // 5. increment group id
        this->groupID++;
    }
}  // namespace Observer
