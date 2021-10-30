#pragma once

#include "ValidatorBySufficientSamples.hpp"
#include "BaseEventValidator.hpp"

#include <iostream>
#include <vector>

namespace Observer {
    class ValidatorBySufficientSamples : public ValidatorHandler {
    public:
        ValidationResult isValid(RawCameraEvent& request, ValidationResult &result) override;
    };
}