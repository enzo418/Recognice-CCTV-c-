#pragma once

#include "../../Domain/Event/CameraEvent.hpp"
#include "../../Domain/Validators/ValidationResult.hpp"
#include "../ChainOfResponsibilityBasics.hpp"

namespace Observer {
    using IValidatorHandler = Handler<ValidationResult, CameraEvent&>;
}