#pragma once

#include "../../Size.hpp"

namespace Observer {
    struct OutputPreviewConfiguration {
        bool showOutput;
        Size resolution;
        double scaleFactor;
        bool showIgnoredAreas;
        bool showProcessedFrames;

        bool operator==(const OutputPreviewConfiguration&) const = default;
    };
}  // namespace Observer
