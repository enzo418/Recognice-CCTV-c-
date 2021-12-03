#pragma once

#include <iostream>
#include <vector>

#include "BaseEventValidator.hpp"
#include "ValidatorBySufficientSamples.hpp"

namespace Observer {
    template <typename TFrame>
    class ValidatorBySufficientSamples : public ValidatorHandler<TFrame> {
       public:
        ValidationResult<TFrame> isValid(
            RawCameraEvent<TFrame>& request,
            ValidationResult<TFrame>& result) override;
    };
}  // namespace Observer