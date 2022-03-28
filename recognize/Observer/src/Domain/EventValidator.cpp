#include "EventValidator.hpp"

namespace Observer {

    EventValidator::EventValidator(CameraConfiguration* pCameraCfg) {
        this->cameraCfg = pCameraCfg;

        auto validatorByBlobs =
            new ValidatorByBlobs(this->cameraCfg->blobDetection);
        //        validatorByBlobs.SetNext()
        handlers.push_back(validatorByBlobs);

        this->handler = handlers[0];
    }

    EventValidator::~EventValidator() {
        for (auto& h : this->handlers) {
            delete h;
        }

        this->handlers.clear();
    }

    void EventValidator::InternalStart() {
        while (this->running) {
            // wait until there is at least 1 item in the pool
            this->smpQueue.acquire();

            // declare event and configuration
            CameraEvent rawCameraEvent;
            CameraConfiguration* cfg;

            // get next pool item
            std::tie(cfg, rawCameraEvent) =
                std::move(this->validationPool.pop());

            OBSERVER_TRACE("New event from camera '{}' received, analyzing it.",
                           cfg->name);

            // validate the event
            ValidationResult result = this->handler->Handle(rawCameraEvent);

            // send the event
            if (result.IsValid()) {
                OBSERVER_TRACE("Event from camera '{}' was valid", cfg->name);

                Event& event = result.GetEvent();

                // set camera name
                event.SetCameraName(cfg->name);

                // notify all the subscribers with the event
                this->eventPublisher.notifySubscribers(
                    std::move(event), std::move(rawCameraEvent));
            } else {
                OBSERVER_TRACE(
                    "Event from camera '{0}' was not valid due to {1}.",
                    cfg->name,
                    std::accumulate(result.GetMessages().begin(),
                                    result.GetMessages().end(), std::string(),
                                    [](const std::string& a,
                                       const std::string& b) -> std::string {
                                        return a + (a.length() > 0 ? "," : "") +
                                               b;
                                    }));
            }
        }
    }

    void EventValidator::Add(CameraConfiguration* cfg, CameraEvent ev) {
        this->validationPool.push(std::make_pair(cfg, std::move(ev)));

        // add 1 item to poll
        this->smpQueue.release();
    }

    void EventValidator::update(CameraConfiguration* cam, CameraEvent ev) {
        this->Add(cam, std::move(ev));
    }

    void EventValidator::SubscribeToEventValidationDone(
        IEventSubscriber* subscriber) {
        this->eventPublisher.subscribe(subscriber);
    }
}  // namespace Observer