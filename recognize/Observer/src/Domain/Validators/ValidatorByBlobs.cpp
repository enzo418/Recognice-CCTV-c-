
#include "ValidatorByBlobs.hpp"

namespace Observer {
    ValidatorByBlobs::ValidatorByBlobs(
        const BlobDetectionConfiguration& pDetectorCfg)
        : contoursDetector(pDetectorCfg.thresholdingParams,
                           pDetectorCfg.contoursFilters),
          blobDetector(pDetectorCfg.blobDetectorParams,
                       pDetectorCfg.blobFilters, this->contoursDetector),
          ValidatorHandler() {}

    ValidationResult ValidatorByBlobs::isValid(CameraEvent& request) {
        auto frames = request.GetFrames();
        const Size targetSize = frames[0].GetSize();
        contoursDetector.SetScale(targetSize);
        blobDetector.SetScale(targetSize);

        auto contours = this->contoursDetector.FindContours(frames);

        if (contours.size() > 0) {
            auto blobs = this->blobDetector.FindBlobs(contours);

            if (blobs.size() > 0) {
                OBSERVER_INFO("Valid notification, blobs size: {0}",
                              blobs.size());

                return ValidationResult(true, {}, std::move(blobs));
            } else {
                return ValidationResult(false,
                                        {"Couldn't find significant blobs."});
            }
        } else {
            return ValidationResult(false, {"Not enough contours"});
        }
    }
}  // namespace Observer