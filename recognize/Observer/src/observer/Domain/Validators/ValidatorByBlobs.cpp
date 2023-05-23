
#include "ValidatorByBlobs.hpp"

#include "observer/Instrumentation/Instrumentation.hpp"

namespace Observer {
    ValidatorByBlobs::ValidatorByBlobs(
        const BlobDetectionConfiguration& pDetectorCfg)
        : ValidatorHandler(),
          contoursDetector(pDetectorCfg.thresholdParams,
                           pDetectorCfg.contoursFilters),
          blobDetector(pDetectorCfg.blobDetectorParams,
                       pDetectorCfg.blobFilters, this->contoursDetector) {}

    void ValidatorByBlobs::isValid(CameraEvent& request, Result& result) {
        OBSERVER_SCOPE("Validate event by blobs");

        auto frames = request.GetFrames();

        OBSERVER_TRACE("Validation frames count: {0}", frames.size());

        if (frames.size() == 0) {
            result.SetValid(false);
            result.AddMessages({"No frames to validate."});
            return;
        }

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