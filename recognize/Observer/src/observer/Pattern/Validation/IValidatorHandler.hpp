#pragma once

#include "../ChainOfResponsibilityBasics.hpp"
#include "observer/Domain/Event/CameraEvent.hpp"
#include "observer/Domain/Validators/ValidationResult.hpp"

namespace Observer {
    using IValidatorHandler = Handler<void, CameraEvent&, ValidationResult&>;
}