#pragma once

#include "BlobDetector/BlobDetectorParams.hpp"
#include "BlobDetector/BlobFilters.hpp"
#include "Contours/ContoursFilters.hpp"
#include "FramesProcessor/ThresholdingParams.hpp"

namespace Observer {
    struct BlobDetectionConfiguration {
        bool enabled {true};

        BlobDetectorParams blobDetectorParams;
        BlobFilters blobFilters;
        ContoursFilter contoursFilters;
        ThresholdParams thresholdParams;

        bool operator==(const BlobDetectionConfiguration&) const = default;
    };
}  // namespace Observer