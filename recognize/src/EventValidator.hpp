#pragma once

#include "ValidatorBySufficientSamples.hpp"
#include "Semaphore.hpp"
#include "SimpleBlockingQueue.hpp"
#include "BaseEventValidator.hpp"
#include "BaseCameraEvent.hpp"
#include "InterfaceFunctionality.hpp"

namespace Observer {
    class EventValidator : public CameraEventSubscriber, public IFunctionality {
    public:
        EventValidator();

        void Add(CameraConfiguration* cfg, RawCameraEvent ev);

        void Start() override;

        void Stop() override;

        void SubscribeToEventValidationDone(CameraEventSubscriber* subscriber);

        void update(CameraConfiguration* cfg, RawCameraEvent ev) override;

        ~EventValidator();

    private:
        bool running;

        Semaphore smpQueue;

        IValidatorHandler* handler;
        std::vector<IValidatorHandler*> handlers;

        SimpleBlockingQueue<std::pair<CameraConfiguration*, RawCameraEvent>> validationPool;

        CameraEventPublisher eventPublisher;
    };
}