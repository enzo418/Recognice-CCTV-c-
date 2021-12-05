#pragma once

#include "../ChainOfResponsibilityBasics.hpp"
#include "../../Domain/Validators/ValidationResult.hpp"
#include "../../Domain/Event/CameraEvent.hpp"

namespace Observer {
    template<typename TFrame>
    using IValidatorHandler = Handler<ValidationResult<TFrame>, CameraEvent<TFrame>&>;
}