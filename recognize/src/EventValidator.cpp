#include "EventValidator.hpp"

namespace Observer {
    EventValidator::EventValidator() {
        auto validatorBySufficientSamples = new ValidatorBySufficientSamples();
        //        validatorBySufficientSamples.SetNext()
        handlers.push_back(validatorBySufficientSamples);
    }

    EventValidator::~EventValidator() {
        for (auto& h : this->handlers) {
            delete h;
        }

        this->handlers.clear();
    }

    void EventValidator::Start() {
        this->running = true;
        while (this->running) {
            // wait until there is at least 1 item in the pool
            this->smpQueue.acquire();

            // declare event and configuration
            RawCameraEvent rawCameraEvent;
            CameraConfiguration* cfg;

            // get next pool item
            std::tie(cfg, rawCameraEvent) =
                std::move(this->validationPool.pop());

            // validate the event
            ValidationResult result;
            this->handler->Handle(rawCameraEvent, result);

            // send the event
            if (result.IsValid()) {
                Event& event = result.GetEvent();

                // set camera name
                event.SetCameraName(cfg->name);

                // notify all the subscribers with the event
                this->eventPublisher.notifySubscribers(
                    std::move(event), std::move(rawCameraEvent));
            } else {
                // TODO: Log the result.message
            }
        }
    }

    void EventValidator::Add(CameraConfiguration* cfg, RawCameraEvent ev) {
        // add 1 item to poll
        this->smpQueue.release();
        this->validationPool.push(std::make_pair(cfg, std::move(ev)));
    }

    void EventValidator::update(CameraConfiguration* cam, RawCameraEvent ev) {
        this->Add(cam, std::move(ev));
    }

    void EventValidator::Stop() { this->running = false; }

    void EventValidator::SubscribeToEventValidationDone(
        ISubscriber<Event, RawCameraEvent>* subscriber) {
        this->eventPublisher.subscribe(subscriber);
    }
}  // namespace Observer