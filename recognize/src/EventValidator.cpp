#include "EventValidator.hpp"

namespace Observer {
    EventValidator::EventValidator() {
        auto validatorBySufficientSamples = new ValidatorBySufficientSamples();
//        validatorBySufficientSamples.SetNext()
        handlers.push_back(validatorBySufficientSamples);
    }

    void EventValidator::Start() {
        this->running = true;
        while (this->running) {
            this->smpQueue.acquire();
            auto ev = this->validationPool.pop();
            ValidationResult result;
            this->handler->Handle(ev, result);
            if (result.valid) {
                // TODO: Publish event
            } else {
                // TODO: Log the result.message
            }
        }
    }

    void EventValidator::Add(RawCameraEvent &ev) {
        this->smpQueue.release();
        this->validationPool.push(ev);
    }

    void EventValidator::Stop() {
        this->running = false;
    }

    EventValidator::~EventValidator() {
        for(auto& h : this->handlers) {
            delete h;
        }

        this->handlers.clear();
    }
}