#include "NotificationsController.hpp"

namespace Observer {

    NotificationsController::NotificationsController(Configuration* cfg) {
        this->config = cfg;
        this->groupID = 0;

        if (cfg->localWebConfiguration.enabled) {
            auto ptrLocal = new LocalNotifications(cfg->localWebConfiguration);

            this->localNotifications = ptrLocal;

            this->AddService(ptrLocal, &this->config->localWebConfiguration);
        }

        if (cfg->telegramConfiguration.enabled) {
            auto ptrTelegram =
                new TelegramNotifications(&cfg->telegramConfiguration);

            this->AddService(ptrTelegram, &this->config->telegramConfiguration);
        }

        this->running = false;
    }

    NotificationsController::~NotificationsController() {
        // delete all the memory allocated here
        for (auto sp : this->services) {
            delete sp;
        }
        this->services.clear();
    }

    void NotificationsController::Send(TextNotification notification) {
        // 1. For each service call Send(TextNotification)
        for (auto&& service : this->services) {
            if (this->servicesType[{service,
                                    flag_to_int(ENotificationType::TEXT)}]) {
                service->SendText(DTONotification(notification.GetGroupID(),
                                                  notification.GetCaption(),
                                                  ENotificationType::TEXT));
            }
        }
    }

    void NotificationsController::Send(ImageNotification notification) {
        Size imSize = notification.GetImage().GetSize();

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
                service->SendImage(DTONotification(
                    notification.GetGroupID(), notification.GetCaption(),
                    ENotificationType::IMAGE, path));
            }
        }

        auto& servD = this->drawableServices[flag_to_int(ETrazable::IMAGE)];
        // guard: if there is at leat 1 service that need the trace
        if (!servD.empty()) {
            // 3. Draw trace on image
            ImageDrawBlob::Get().DrawBlobs(
                notification.GetImage(), notification.GetEvent().GetBlobs(),
                notification.GetEvent().GetFirstFrameWhereFindingWasFound(),
                factor, factor);

            const auto path_trace =
                notification.BuildNotification(this->config->mediaFolderPath);

            // 4. For each service that need the trace call
            // SendVideo(image2path)
            for (auto&& service : servD) {
                service->SendImage(DTONotification(
                    notification.GetGroupID(), notification.GetCaption(),
                    ENotificationType::IMAGE, path_trace));
            }
        }
    }

    void NotificationsController::Send(VideoNotification notification) {
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
                service->SendVideo(DTONotification(
                    notification.GetGroupID(), notification.GetCaption(),
                    ENotificationType::VIDEO, videoPath));
            }
        }

        // guard: if there is at leat 1 service that need the trace
        if (!this->drawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 3. Draw trace on video

            OBSERVER_INFO("Drawing blobs on notification");
            ImageDrawBlob::Get().DrawBlobs(notification.GetFrames(),
                                           notification.GetEvent().GetBlobs(),
                                           factor, factor);

            const auto videoPath =
                notification.BuildNotification(this->config->mediaFolderPath);

            // 4. For each service that need the trace call
            // SendVideo(video2path)
            for (auto&& service :
                 this->drawableServices[flag_to_int(ETrazable::VIDEO)]) {
                service->SendVideo(DTONotification(
                    notification.GetGroupID(), notification.GetCaption(),
                    ENotificationType::VIDEO, videoPath));
            }
        }
    }

    void NotificationsController::AddNotification(TextNotification textNotf) {
        this->textQueue.push(textNotf);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    void NotificationsController::AddNotification(ImageNotification imageNotf) {
        this->imageQueue.push(imageNotf);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    void NotificationsController::AddNotification(VideoNotification videoNotf) {
        this->videoQueue.push(videoNotf);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    void NotificationsController::ConsumeNotifications() {
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

    void NotificationsController::InternalStart() {
        this->ConsumeNotifications();
    }

    void NotificationsController::AddService(
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

    void NotificationsController::update(Event event,
                                         CameraEvent rawCameraEvent) {
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
        ImageNotification imageNotification(
            this->groupID, event, cameraName,
            rawCameraEvent.GetFrameAt(indexFirst));

        // 3. Create a video notification using the frames
        VideoNotification videoNotification(
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

    void NotificationsController::SubscribeToNewNotifications(
        INotificationEventSubscriber* subscriber) {
        if (localNotifications != nullptr) {
            localNotifications->SubscribeToNewNotifications(subscriber);
        }
    }
}  // namespace Observer
