#pragma once

#include "../../Domain/Event/CameraEvent.hpp"
#include "../../Domain/Validators/ValidationResult.hpp"
#include "../ChainOfResponsibilityBasics.hpp"

namespace Observer {
    template <typename TFrame>
    using IValidatorHandler =
        Handler<ValidationResult<TFrame>, CameraEvent<TFrame>&>;
}