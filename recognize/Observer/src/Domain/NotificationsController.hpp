#pragma once

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>

#include "../Blob/BlobGraphics.hpp"
#include "../IFunctionality.hpp"
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
#include "Notification/RestClientLocalWebNotifications.hpp"
#include "Notification/TelegramNotifications.hpp"

namespace Observer {
    class INotificationEventSubscriber : public ISubscriber<DTONotification> {
        void update(DTONotification ev) override = 0;
    };
}  // namespace Observer

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

        void SubscribeToNewNotifications(
            INotificationEventSubscriber* subscriber);

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
        SimpleBlockingQueue<ImageNotification<TFrame>> imageQueue;
        SimpleBlockingQueue<VideoNotification<TFrame>> videoQueue;
        int groupID;

        Publisher<DTONotification> notificationsPublisher;
    };

    template <typename TFrame>
    NotificationsController<TFrame>::NotificationsController(
        Configuration* cfg) {
        this->config = cfg;
        this->groupID = 0;

        if (cfg->localWebConfiguration.enabled) {
            auto ptrLocal =
                new RestClientLocalWebNotifications(cfg->localWebConfiguration);

            this->AddService(ptrLocal, &this->config->localWebConfiguration);
        }

        if (cfg->telegramConfiguration.enabled) {
            auto ptrTelegram =
                new TelegramNotifications(&cfg->telegramConfiguration);

            this->AddService(ptrTelegram, &this->config->telegramConfiguration);
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
            if (this->servicesType[{service,
                                    flag_to_int(ENotificationType::TEXT)}]) {
                service->SendText(DTONotification(notification.GetGroupID(),
                                                  notification.GetCaption()));
            }
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Send(
        ImageNotification<TFrame> notification) {
        Size imSize =
            ImageTransformation<TFrame>::GetSize(notification.GetImage());

        if (imSize.width == 0 || imSize.height == 0) {
            OBSERVER_ERROR("Trying to send an empty image. Caption: {}",
                           notification.GetCaption());
            return;
        }

        const double factor =
            ((double)config->resizeNotifications.image / 100.0);
        notification.Resize(factor, factor);

        if (!this->notDrawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 1. Build
            const auto path =
                notification.BuildNotification(this->config->mediaFolderPath);

            for (auto&& service :
                 this->notDrawableServices[flag_to_int(ETrazable::IMAGE)]) {
                service->SendImage(DTONotification(notification.GetGroupID(),
                                                   notification.GetCaption(),
                                                   path));
            }
        }

        auto& servD = this->drawableServices[flag_to_int(ETrazable::IMAGE)];
        // guard: if there is at leat 1 service that need the trace
        if (!servD.empty()) {
            // 3. Draw trace on image
            BlobGraphics<TFrame>::DrawBlobs(
                notification.GetImage(), notification.GetEvent().GetBlobs(),
                notification.GetEvent().GetFirstFrameWhereFindingWasFound(),
                factor, factor);

            const auto path_trace =
                notification.BuildNotification(this->config->mediaFolderPath);

            // 4. For each service that need the trace call
            // SendVideo(image2path)
            for (auto&& service : servD) {
                service->SendImage(DTONotification(notification.GetGroupID(),
                                                   notification.GetCaption(),
                                                   path_trace));
            }
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::Send(
        VideoNotification<TFrame> notification) {
        auto& frames = notification.GetFrames();

        if (frames.empty()) {
            OBSERVER_CRITICAL(
                "Empty frames while sendind a video notification. Video error "
                "data: GID={0}, CAM_NAME={1}, N_BLOBS={2}",
                notification.GetGroupID(),
                notification.GetEvent().GetCameraName(),
                notification.GetEvent().GetBlobs().size());
            return;
        }

        const double factor =
            ((double)config->resizeNotifications.video / 100.0);
        notification.Resize(factor, factor);

        if (!this->notDrawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 1. Build
            const auto videoPath =
                notification.BuildNotification(this->config->mediaFolderPath);

            // 2. For each service that doesn't need the trace call
            // SendVideo(videopath)
            for (auto&& service :
                 this->notDrawableServices[flag_to_int(ETrazable::VIDEO)]) {
                service->SendVideo(DTONotification(notification.GetGroupID(),
                                                   notification.GetCaption(),
                                                   videoPath));
            }
        }

        // guard: if there is at leat 1 service that need the trace
        if (!this->drawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 3. Draw trace on video

            OBSERVER_INFO("Drawing blobs on notification");
            BlobGraphics<TFrame>::DrawBlobs(notification.GetFrames(),
                                            notification.GetEvent().GetBlobs(),
                                            factor, factor);

            const auto videoPath =
                notification.BuildNotification(this->config->mediaFolderPath);

            // 4. For each service that need the trace call
            // SendVideo(video2path)
            for (auto&& service :
                 this->drawableServices[flag_to_int(ETrazable::VIDEO)]) {
                service->SendVideo(DTONotification(notification.GetGroupID(),
                                                   notification.GetCaption(),
                                                   videoPath));
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
    void NotificationsController<TFrame>::AddService(
        IMessagingService* service, NotificationsServiceConfiguration* cfg) {
        // TODO: iterate enums

        static const ETrazable trazable_types[] = {ETrazable::IMAGE,
                                                   ETrazable::VIDEO};

        static const std::unordered_map<ETrazable, ENotificationType> equiv = {
            {ETrazable::IMAGE, ENotificationType::IMAGE},
            {ETrazable::VIDEO, ENotificationType::VIDEO}};

        ENotificationType typesAccepted = cfg->noticationsToSend;

        this->services.push_back(service);

        for (const auto type : Helpers::Notifications::NOTIFICATION_TYPES) {
            if (has_flag(typesAccepted, type)) {
                this->servicesType[{service, flag_to_int(type)}] = true;
            } else {
                this->servicesType[{service, flag_to_int(type)}] = false;
            }
        }

        for (const auto type : trazable_types) {
            if (this->servicesType[{service, flag_to_int(equiv.at(type))}]) {
                if (has_flag(cfg->drawTraceOfChangeOn, type)) {
                    drawableServices[flag_to_int(type)].push_back(service);
                } else {
                    notDrawableServices[flag_to_int(type)].push_back(service);
                }
            }
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

        videoNotification.SetFrameRate(rawCameraEvent.GetFrameRate());
        videoNotification.SetFrameSize(rawCameraEvent.GetFramesSize());

        // 4. call AddNotification for each one
        this->AddNotification(textNotification);
        this->AddNotification(imageNotification);
        this->AddNotification(videoNotification);

        // 5. increment group id
        this->groupID++;

        Size size = rawCameraEvent.GetFramesSize();
        double maxDiagonalDist =
            sqrt(size.width * size.width + size.height * size.height) * 0.01;
        OBSERVER_TRACE("Blobs data: Max={0}", maxDiagonalDist);
        /// TODO: Remove - Debug

        for (auto& blob : event.GetBlobs()) {
            OBSERVER_TRACE(
                "\t- ID: {0}\n\t\tAppearances: {1}\n\t\tAverage Vel: "
                "{2}\n\t\tDistance Traveled: {3} units",
                blob.GetId(), blob.GetAppearances().size(),
                blob.GetAverageMagnitude() / maxDiagonalDist,
                blob.GetDistanceTraveled() / maxDiagonalDist);
        }
    }

    template <typename TFrame>
    void NotificationsController<TFrame>::SubscribeToNewNotifications(
        INotificationEventSubscriber* subscriber) {
        this->notificationsPublisher.subscribe(subscriber);
    }
}  // namespace Observer
