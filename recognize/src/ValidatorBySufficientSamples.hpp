#pragma once

#include <iostream>
#include <vector>

#include "BaseEventValidator.hpp"
#include "ValidatorBySufficientSamples.hpp"

namespace Observer {
    class ValidatorBySufficientSamples : public ValidatorHandler {
       public:
        ValidationResult isValid(RawCameraEvent& request,
                                 ValidationResult& result) override;
    };
}  // namespace Observer