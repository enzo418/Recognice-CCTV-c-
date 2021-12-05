#pragma once

#include "../../Pattern/ChainOfResponsibilityBasics.hpp"
#include "../Event/CameraEvent.hpp"
#include "../Event/Event.hpp"
#include "ValidationResult.hpp"

namespace Observer {
    template <typename TFrame>
    class ValidatorHandler : public AbstractHandler<ValidationResult<TFrame>,
                                                    CameraEvent<TFrame>&> {
       public:
        ValidationResult<TFrame> Handle(
            CameraEvent<TFrame>& request,
            ValidationResult<TFrame>& result) override {
            auto res = this->isValid(request, result);
            if (res.IsValid()) {
                return res;
            } else {
                return AbstractHandler<ValidationResult<TFrame>,
                                       CameraEvent<TFrame>&>::Handle(request,
                                                                     result);
            }
        }

        virtual ValidationResult<TFrame> isValid(
            CameraEvent<TFrame>& request, ValidationResult<TFrame>& result) = 0;
    };
}  // namespace Observer