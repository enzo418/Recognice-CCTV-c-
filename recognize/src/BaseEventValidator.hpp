#pragma once

#include "BaseChainOfResponsibility.hpp"
#include "RawCameraEvent.hpp"

namespace Observer {
    using IValidatorHandler = Handler<ValidationResult, RawCameraEvent&>;

    /**
     * BaseConcreteHandler
     */
    class ValidatorHandler : public AbstractHandler<ValidationResult, RawCameraEvent&> {
    public:
        ValidationResult Handle(RawCameraEvent& request, ValidationResult &result) override {
            auto res = this->isValid(request, result);
            if (res.valid) {
                return res;
            } else {
                return AbstractHandler<ValidationResult, RawCameraEvent&>::Handle(request, result);
            }
        }

        virtual ValidationResult isValid(RawCameraEvent& request, ValidationResult &result) = 0;
    };
}