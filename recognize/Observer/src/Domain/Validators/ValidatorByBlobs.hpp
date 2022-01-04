#pragma once

#include <iostream>
#include <vector>

#include "../../Blob/BlobDetector/BlobDetector.hpp"
#include "../../Blob/Contours/ContoursDetector.hpp"
#include "../../Blob/Contours/ContoursTypes.hpp"
#include "ValidatorHandler.hpp"

namespace Observer {
    template <typename TFrame>
    class ValidatorByBlobs : public ValidatorHandler<TFrame> {
       public:
        ValidatorByBlobs(const BlobDetectorParams& pDetectorParams,
                         const BlobFilters& blobFilters,
                         const ThresholdingParams& contoursParams,
                         const ContoursFilter& filters)
            : blobDetector(pDetectorParams, blobFilters),
              contoursDetector(contoursParams, filters),
              ValidatorHandler<TFrame>() {}

       public:
        ValidationResult<TFrame> isValid(CameraEvent<TFrame>& request) override;

       private:
        ContoursDetector<TFrame> contoursDetector;
        BlobDetector<TFrame> blobDetector;
    };

    template <typename TFrame>
    ValidationResult<TFrame> ValidatorByBlobs<TFrame>::isValid(
        CameraEvent<TFrame>& request) {
        auto contours =
            this->contoursDetector.FindContours(request.GetFrames());

        if (contours.size() > 0) {
            auto blobs = this->blobDetector.FindBlobs(contours);

            if (blobs.size() > 0) {
                return ValidationResult<TFrame>(true, {}, std::move(blobs));
            } else {
                return ValidationResult<TFrame>(
                    false, {"Couldn't find significant blobs."});
            }
        } else {
            return ValidationResult<TFrame>(false, {"Not enough contours"});
        }
    }
}  // namespace Observer