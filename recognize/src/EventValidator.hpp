#pragma once

#include "ValidatorBySufficientSamples.hpp"
#include "Semaphore.hpp"
#include "SimpleBlockingQueue.hpp"
#include "BaseEventValidator.hpp"

namespace Observer {
    class EventValidator {
    public:
        EventValidator();

        void Add(RawCameraEvent &ev);

        void Start();

        void Stop();

        ~EventValidator();

    private:
        bool running;

        Semaphore smpQueue;

        IValidatorHandler* handler;
        std::vector<IValidatorHandler*> handlers;

        SimpleBlockingQueue<RawCameraEvent> validationPool;
    };
}