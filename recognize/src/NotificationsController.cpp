#include "NotificationsController.hpp"

namespace Observer {
    NotificationsController::NotificationsController(Configuration* cfg) {
        this->config = cfg;
        this->groupID = 0;

        auto ptrLocal = new RestClientLocalWebNotifications(
            cfg->localWebConfiguration.webServerUrl);
        auto ptrTelegram = new TelegramNotifications();

        this->AddServiceToDrawable(ptrLocal,
                                   &this->config->localWebConfiguration);
        this->AddServiceToDrawable(ptrTelegram,
                                   &this->config->telegramConfiguration);

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
            service->SendText(notification.GetCaption());
        }
    }

    void NotificationsController::Send(ImageNotification notification) {
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

    void NotificationsController::Send(VideoNotification notification) {
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

            // 2. n = repository.get() - Get the prioritary notification (Text >
            // Image > Video)
            // 3. this->send(n)

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

    void NotificationsController::Start() {
        this->running = true;
        this->ConsumeNotifications();
    }

    void NotificationsController::Stop() { this->running = false; }

    void NotificationsController::AddServiceToDrawable(
        MessagingService* service, NotificationsServiceConfiguration* cfg) {
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

    void NotificationsController::update(Event event,
                                         RawCameraEvent rawCameraEvent) {
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

        // 4. call AddNotification for each one
        this->AddNotification(textNotification);
        this->AddNotification(imageNotification);
        this->AddNotification(videoNotification);

        // 5. increment group id
        this->groupID++;
    }

}  // namespace Observer
