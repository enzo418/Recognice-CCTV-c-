
#include "ValidatorByBlobs.hpp"

namespace Observer {
    ValidatorByBlobs::ValidatorByBlobs(
        const BlobDetectionConfiguration& pDetectorCfg)
        : ValidatorHandler(),
          contoursDetector(pDetectorCfg.thresholdParams,
                           pDetectorCfg.contoursFilters),
          blobDetector(pDetectorCfg.blobDetectorParams,
                       pDetectorCfg.blobFilters, this->contoursDetector) {}

    void ValidatorByBlobs::isValid(CameraEvent& request, Result& result) {
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
                result.SetValid(true);
                result.SetBlobs(std::move(blobs));
            } else {
                result.SetValid(false);
                result.AddMessages({"Couldn't find significant blobs."});
            }
        } else {
            result.SetValid(false);
            result.AddMessages({"Couldn't find contours."});
        }
    }
}  // namespace Observer