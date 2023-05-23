#include "NotificationsController.hpp"

#include "observer/Domain/Classification/BlobClassification.hpp"

namespace Observer {

    NotificationsController::NotificationsController(Configuration* cfg) {
        this->config = cfg;

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
                service->SendText(DTONotification(
                    notification.GetGroupID(), notification.GetContent(),
                    ENotificationType::TEXT,
                    notification.GetEvent().GetCameraName()));
            }
        }
    }

    void NotificationsController::Send(ImageNotification notification) {
        Size imSize = notification.GetImage().GetSize();

        if (imSize.width == 0 || imSize.height == 0) {
            OBSERVER_ERROR("Trying to send an empty image. Content: {}",
                           notification.GetContent());
            return;
        }

        const double factor =
            ((double)config->resizeNotifications.image / 100.0);
        notification.Resize(factor, factor);

        if (!this->notDrawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 1. Build
            notification.BuildNotification();

            for (auto&& service :
                 this->notDrawableServices[flag_to_int(ETrazable::IMAGE)]) {
                service->SendImage(DTONotification(
                    notification.GetGroupID(), notification.GetContent(),
                    ENotificationType::IMAGE,
                    notification.GetEvent().GetCameraName()));
            }
        }

        auto& servD = this->drawableServices[flag_to_int(ETrazable::IMAGE)];
        // guard: if there is at least 1 service that need the trace
        if (!servD.empty()) {
            // 3. Draw trace on image
            ImageDrawBlob::Get().DrawBlobs(
                notification.GetImage(), notification.GetEvent().GetBlobs(),
                notification.GetEvent().GetClassifications(),
                notification.GetEvent().GetFirstFrameWhereFindingWasFound(),
                factor, factor);

            notification.BuildNotification();

            // 4. For each service that need the trace call
            // SendVideo(image2path)
            for (auto&& service : servD) {
                service->SendImage(DTONotification(
                    notification.GetGroupID(), notification.GetContent(),
                    ENotificationType::IMAGE,
                    notification.GetEvent().GetCameraName()));
            }
        }
    }

    void NotificationsController::Send(VideoNotification notification) {
        auto& frames = notification.GetFrames();

        if (frames.empty()) {
            OBSERVER_CRITICAL(
                "Empty frames while sending a video notification. Video error "
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
            notification.BuildNotification();

            // 2. For each service that doesn't need the trace call
            // SendVideo(video path)
            for (auto&& service :
                 this->notDrawableServices[flag_to_int(ETrazable::VIDEO)]) {
                service->SendVideo(DTONotification(
                    notification.GetGroupID(), notification.GetContent(),
                    ENotificationType::VIDEO,
                    notification.GetEvent().GetCameraName()));
            }
        }

        // guard: if there is at least 1 service that need the trace
        if (!this->drawableServices[flag_to_int(ETrazable::VIDEO)].empty()) {
            // 3. Draw trace on video

            BlobClassifications classifications =
                notification.GetEvent().GetClassifications();

            OBSERVER_INFO("Drawing blobs on notification");
            ImageDrawBlob::Get().DrawBlobs(notification.GetFrames(),
                                           notification.GetEvent().GetBlobs(),
                                           classifications, factor, factor);

            notification.BuildNotification();

            // 4. For each service that need the trace call
            // SendVideo(video2path)
            for (auto&& service :
                 this->drawableServices[flag_to_int(ETrazable::VIDEO)]) {
                service->SendVideo(DTONotification(
                    notification.GetGroupID(), notification.GetContent(),
                    ENotificationType::VIDEO,
                    notification.GetEvent().GetCameraName()));
            }
        }
    }

    void NotificationsController::AddNotification(TextNotification textN) {
        this->textQueue.push(textN);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    void NotificationsController::AddNotification(ImageNotification imageN) {
        this->imageQueue.push(imageN);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    void NotificationsController::AddNotification(VideoNotification videoN) {
        this->videoQueue.push(videoN);

        // Semaphore - Added 1 notification
        this->smpQueue.release();
    }

    void NotificationsController::ConsumeNotifications() {
        while (this->running) {
            // 1. semaphore acquire - There is at least 1 notification
            if (smpQueue.acquire_timeout<250>()) {
                /// TODO: Improve quality with a unordered_map with the queues,
                /// like drawable
                if (this->textQueue.size() > 0) {
                    this->Send(this->textQueue.pop());
                } else if (this->imageQueue.size() > 0) {
                    this->Send(this->imageQueue.pop());
                } else if (this->videoQueue.size() > 0) {
                    this->Send(this->videoQueue.pop());
                }
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

        ENotificationType typesAccepted = cfg->notificationsToSend;

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

    void NotificationsController::update(
        EventDescriptor& event, std::shared_ptr<CameraEvent> rawCameraEvent) {
        const int groupID = rawCameraEvent->GetGroupID();

        // create destination folders
        const auto imagesFolder =
            (std::filesystem::path(this->config->mediaFolderPath) / "images")
                .string();
        if (!std::filesystem::exists(imagesFolder)) {
            std::filesystem::create_directories(imagesFolder);
        }

        const auto videosFolder =
            (std::filesystem::path(this->config->mediaFolderPath) / "videos")
                .string();

        if (!std::filesystem::exists(videosFolder)) {
            std::filesystem::create_directories(videosFolder);
        }

        // 1. Create a text notification
        // 1. a. Get camera name
        std::string cameraName = event.GetCameraName();

        // 1. b. Replace template text notification
        std::string text = SpecialFunctions::FormatNotificationTextString(
            this->config->notificationTextTemplate, cameraName);

        // 1. c. Create the notification
        TextNotification textNotification(groupID, event, text);

        // 2. Create an image notification using the first frame where the event
        // happen

        int indexFirst = event.GetFirstFrameWhereFindingWasFound();
        ImageNotification imageNotification(
            groupID, event, rawCameraEvent->GetFrameAt(indexFirst),
            imagesFolder);

        // 3. Create a video notification using the frames
        VideoNotification videoNotification(
            groupID, event, rawCameraEvent->PopFrames(), videosFolder);

        videoNotification.SetFrameRate(rawCameraEvent->GetFrameRate());
        videoNotification.SetFrameSize(rawCameraEvent->GetFramesSize());

        // 4. call AddNotification for each one
        this->AddNotification(textNotification);
        this->AddNotification(imageNotification);
        this->AddNotification(videoNotification);

        Size size = rawCameraEvent->GetFramesSize();
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
