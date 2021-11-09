#pragma once

#include "BaseCameraEvent.hpp"
#include "BaseEventValidator.hpp"
#include "Event.hpp"
#include "IFunctionality.hpp"
#include "Semaphore.hpp"
#include "SimpleBlockingQueue.hpp"
#include "ValidatorBySufficientSamples.hpp"

namespace Observer {
    class EventValidator : public CameraEventSubscriber, public IFunctionality {
       public:
        EventValidator();

        void Add(CameraConfiguration* cfg, RawCameraEvent ev);

        void Start() override;

        void Stop() override;

        void SubscribeToEventValidationDone(
            ISubscriber<Event, RawCameraEvent>* subscriber);

        void update(CameraConfiguration* cfg, RawCameraEvent ev) override;

        ~EventValidator();

       private:
        bool running;

        Semaphore smpQueue;

        IValidatorHandler* handler;
        std::vector<IValidatorHandler*> handlers;

        SimpleBlockingQueue<std::pair<CameraConfiguration*, RawCameraEvent>>
            validationPool;

        Publisher<Event, RawCameraEvent> eventPublisher;
    };
}  // namespace Observer