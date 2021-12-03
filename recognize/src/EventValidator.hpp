#pragma once

#include "BaseCameraEvent.hpp"
#include "BaseEventValidator.hpp"
#include "Event.hpp"
#include "IFunctionality.hpp"
#include "Semaphore.hpp"
#include "SimpleBlockingQueue.hpp"
#include "ValidatorBySufficientSamples.hpp"

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
}  // namespace Observer