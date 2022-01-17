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
        typedef ValidationResult<TFrame> Result;

        Result Handle(CameraEvent<TFrame>& request) override {
            auto res = this->isValid(request);
            if (res.IsValid()) {
                return res;
            } else {
                if (this->nextHandler) {
                    return AbstractHandler<
                        Result, CameraEvent<TFrame>&>::Handle(request);
                } else {
                    return res;
                }
            }
        }

        virtual Result isValid(CameraEvent<TFrame>& request) = 0;
    };
}  // namespace Observer