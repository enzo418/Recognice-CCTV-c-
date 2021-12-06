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
        ValidationResult<TFrame> Handle(CameraEvent<TFrame>& request) override {
            auto res = this->isValid(request);
            if (res.IsValid()) {
                return res;
            } else {
                return AbstractHandler<ValidationResult<TFrame>,
                                       CameraEvent<TFrame>&>::Handle(request);
            }
        }

        virtual ValidationResult<TFrame> isValid(
            CameraEvent<TFrame>& request) = 0;
    };
}  // namespace Observer