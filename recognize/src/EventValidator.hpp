#pragma once

#include <numeric>

#include "BaseCameraEvent.hpp"
#include "BaseEventValidator.hpp"
#include "Event.hpp"
#include "IFunctionality.hpp"
#include "Semaphore.hpp"
#include "SimpleBlockingQueue.hpp"
#include "ValidatorBySufficientSamples.hpp"
#include "log/log.hpp"

namespace Observer {
    template <typename TFrame>
    class EventValidator : public CameraEventSubscriber<TFrame>,
                           public IFunctionality {
       public:
        EventValidator();

        void Add(CameraConfiguration* cfg, RawCameraEvent<TFrame> ev);

        void Start() override;

        void Stop() override;

        void SubscribeToEventValidationDone(
            ISubscriber<Event, RawCameraEvent<TFrame>>* subscriber);

        void update(CameraConfiguration* cfg,
                    RawCameraEvent<TFrame> ev) override;

        ~EventValidator();

       private:
        bool running;

        Semaphore smpQueue;

        typename ValidatorHandler<TFrame>::IValidatorHandler* handler;
        std::vector<typename ValidatorHandler<TFrame>::IValidatorHandler*>
            handlers;

        SimpleBlockingQueue<
            std::pair<CameraConfiguration*, RawCameraEvent<TFrame>>>
            validationPool;

        Publisher<Event, RawCameraEvent<TFrame>> eventPublisher;
    };

    template <typename TFrame>
    EventValidator<TFrame>::EventValidator() {
        auto validatorBySufficientSamples =
            new ValidatorBySufficientSamples<TFrame>();
        //        validatorBySufficientSamples.SetNext()
        handlers.push_back(validatorBySufficientSamples);

        this->handler = handlers[0];
    }

    template <typename TFrame>
    EventValidator<TFrame>::~EventValidator() {
        for (auto& h : this->handlers) {
            delete h;
        }

        this->handlers.clear();
    }

    template <typename TFrame>
    void EventValidator<TFrame>::Start() {
        this->running = true;
        while (this->running) {
            // wait until there is at least 1 item in the pool
            this->smpQueue.acquire();

            // declare event and configuration
            RawCameraEvent<TFrame> rawCameraEvent;
            CameraConfiguration* cfg;

            // get next pool item
            std::tie(cfg, rawCameraEvent) =
                std::move(this->validationPool.pop());

            OBSERVER_TRACE("New event from camera '{}' received, analyzing it.",
                           cfg->name);

            // validate the event
            ValidationResult<TFrame> result;
            this->handler->Handle(rawCameraEvent, result);

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
                    "Event from camera '{}' was not valid due to {}.",
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

    template <typename TFrame>
    void EventValidator<TFrame>::Add(CameraConfiguration* cfg,
                                     RawCameraEvent<TFrame> ev) {
        OBSERVER_TRACE("Received event from camera '{}', adding it to the pool",
                       cfg->name);

        // add 1 item to poll
        this->smpQueue.release();
        this->validationPool.push(std::make_pair(cfg, std::move(ev)));
    }

    template <typename TFrame>
    void EventValidator<TFrame>::update(CameraConfiguration* cam,
                                        RawCameraEvent<TFrame> ev) {
        this->Add(cam, std::move(ev));
    }

    template <typename TFrame>
    void EventValidator<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void EventValidator<TFrame>::SubscribeToEventValidationDone(
        ISubscriber<Event, RawCameraEvent<TFrame>>* subscriber) {
        this->eventPublisher.subscribe(subscriber);
    }
}  // namespace Observer