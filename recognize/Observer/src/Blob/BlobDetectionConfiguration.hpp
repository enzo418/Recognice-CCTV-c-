#pragma once

#include "BlobDetector/BlobDetectorParams.hpp"
#include "BlobDetector/BlobFilters.hpp"
#include "Contours/ContoursFilters.hpp"
#include "FramesProcessor/ThresholdingParams.hpp"

namespace Observer {
    struct BlobDetectionConfiguration {
        BlobDetectorParams blobDetectorParams;
        BlobFilters blobFilters;
        ContoursFilter contoursFilters;
        ThresholdingParams thresholdingParams;

        bool operator==(const BlobDetectionConfiguration&) const = default;
    };
}  // namespace Observer