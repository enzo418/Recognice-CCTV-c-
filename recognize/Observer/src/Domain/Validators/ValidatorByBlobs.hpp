#pragma once

#include <iostream>
#include <vector>

#include "../../Blob/BlobDetectionConfiguration.hpp"
#include "../../Blob/BlobDetector/BlobDetector.hpp"
#include "../../Blob/Contours/ContoursDetector.hpp"
#include "../../ImageTransformation.hpp"
#include "ValidatorHandler.hpp"

namespace Observer {
    template <typename TFrame>
    class ValidatorByBlobs : public ValidatorHandler<TFrame> {
       public:
        ValidatorByBlobs(const BlobDetectionConfiguration& pDetectorCfg)
            : contoursDetector(pDetectorCfg.thresholdingParams,
                               pDetectorCfg.contoursFilters),
              blobDetector(pDetectorCfg.blobDetectorParams,
                           pDetectorCfg.blobFilters, this->contoursDetector),
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
        auto frames = request.GetFrames();
        const Size targetSize = ImageTransformation<TFrame>::GetSize(frames[0]);
        contoursDetector.SetScale(targetSize);
        blobDetector.SetScale(targetSize);

        auto contours = this->contoursDetector.FindContours(frames);

        if (contours.size() > 0) {
            auto blobs = this->blobDetector.FindBlobs(contours);

            if (blobs.size() > 0) {
                OBSERVER_INFO("Valid notification, blobs size: {0}",
                              blobs.size());

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