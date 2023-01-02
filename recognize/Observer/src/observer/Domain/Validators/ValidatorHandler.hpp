#pragma once

#include "ValidationResult.hpp"
#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/Domain/Event/EventDescriptor.hpp"
#include "observer/Implementation.hpp"
#include "observer/Pattern/ChainOfResponsibilityBasics.hpp"

namespace Observer {
    class ValidatorHandler
        : public AbstractHandler<ValidationResult, CameraEvent&> {
       public:
        typedef ValidationResult Result;

        Result Handle(CameraEvent& request) override;

        virtual Result isValid(CameraEvent& request) = 0;
    };
}  // namespace Observer