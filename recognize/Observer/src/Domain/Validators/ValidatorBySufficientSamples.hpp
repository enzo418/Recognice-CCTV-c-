#pragma once

#include <iostream>
#include <vector>

#include "ValidatorBySufficientSamples.hpp"
#include "ValidatorHandler.hpp"

namespace Observer {
    template <typename TFrame>
    class ValidatorBySufficientSamples : public ValidatorHandler<TFrame> {
       public:
        ValidationResult<TFrame> isValid(
            CameraEvent<TFrame>& request,
            ValidationResult<TFrame>& result) override;
    };

    template <typename TFrame>
    ValidationResult<TFrame> ValidatorBySufficientSamples<TFrame>::isValid(
        CameraEvent<TFrame>& request, ValidationResult<TFrame>& result) {
        // TODO:
        return {};
    }
}  // namespace Observer