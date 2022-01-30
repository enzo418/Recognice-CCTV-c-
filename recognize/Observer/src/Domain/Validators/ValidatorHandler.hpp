#pragma once

#include "../../Implementation.hpp"
#include "../../Pattern/ChainOfResponsibilityBasics.hpp"
#include "../Event/CameraEvent.hpp"
#include "../Event/Event.hpp"
#include "ValidationResult.hpp"

namespace Observer {
    class ValidatorHandler
        : public AbstractHandler<ValidationResult, CameraEvent&> {
       public:
        typedef ValidationResult Result;

        Result Handle(CameraEvent& request) override;

        virtual Result isValid(CameraEvent& request) = 0;
    };
}  // namespace Observer