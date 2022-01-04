#pragma once

#include <numeric>

#include "../IFunctionality.hpp"
#include "../Log/log.hpp"
#include "../Pattern/Camera/ICameraEventSubscriber.hpp"
#include "../Pattern/Event/IEventSubscriber.hpp"
#include "../Pattern/Validation/IValidatorHandler.hpp"
#include "../Semaphore.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "Event/Event.hpp"
#include "Validators/ValidatorByBlobs.hpp"
#include "Validators/ValidatorHandler.hpp"

namespace Observer {
    template <typename TFrame>
    class EventValidator : public ICameraEventSubscriber<TFrame>,
                           public IFunctionality {
       public:
        EventValidator();

        void Add(CameraConfiguration* cfg, CameraEvent<TFrame> ev);

        void Start() override;

        void Stop() override;

        void SubscribeToEventValidationDone(
            IEventSubscriber<TFrame>* subscriber);

        void update(CameraConfiguration* cfg, CameraEvent<TFrame> ev) override;

        ~EventValidator();

       private:
        bool running;

        Semaphore smpQueue;

        IValidatorHandler<TFrame>* handler;

        std::vector<IValidatorHandler<TFrame>*> handlers;

        SimpleBlockingQueue<
            std::pair<CameraConfiguration*, CameraEvent<TFrame>>>
            validationPool;

        Publisher<Event, CameraEvent<TFrame>> eventPublisher;
    };

    template <typename TFrame>
    EventValidator<TFrame>::EventValidator() {
        // auto validatorByBlobs = new ValidatorByBlobs<TFrame>();
        //        validatorByBlobs.SetNext()
        // handlers.push_back(validatorByBlobs);

        // this->handler = handlers[0];
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
            CameraEvent<TFrame> rawCameraEvent;
            CameraConfiguration* cfg;

            // get next pool item
            std::tie(cfg, rawCameraEvent) =
                std::move(this->validationPool.pop());

            OBSERVER_TRACE("New event from camera '{}' received, analyzing it.",
                           cfg->name);

            // validate the event
            ValidationResult<TFrame> result =
                this->handler->Handle(rawCameraEvent);

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
                                     CameraEvent<TFrame> ev) {
        this->validationPool.push(std::make_pair(cfg, std::move(ev)));

        // add 1 item to poll
        this->smpQueue.release();
    }

    template <typename TFrame>
    void EventValidator<TFrame>::update(CameraConfiguration* cam,
                                        CameraEvent<TFrame> ev) {
        this->Add(cam, std::move(ev));
    }

    template <typename TFrame>
    void EventValidator<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void EventValidator<TFrame>::SubscribeToEventValidationDone(
        IEventSubscriber<TFrame>* subscriber) {
        this->eventPublisher.subscribe(subscriber);
    }
}  // namespace Observer