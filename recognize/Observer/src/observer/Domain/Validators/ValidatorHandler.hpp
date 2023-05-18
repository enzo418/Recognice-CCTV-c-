#pragma once

#include "ValidationResult.hpp"
#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/Domain/Event/EventDescriptor.hpp"
#include "observer/Implementation.hpp"
#include "observer/Pattern/ChainOfResponsibilityBasics.hpp"

namespace Observer {
    class ValidatorHandler
        : public AbstractHandler<void, CameraEvent&, ValidationResult&> {
       public:
        typedef ValidationResult Result;

        void Handle(CameraEvent& request, ValidationResult& result) override;

        virtual void isValid(CameraEvent& request, Result& result) = 0;
    };
}  // namespace Observer