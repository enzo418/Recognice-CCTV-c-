#pragma once

#include "observer/Size.hpp"

namespace Observer {
    struct OutputPreviewConfiguration {
        bool showOutput {true};
        Size resolution {640, 360};
        double scaleFactor {1};

        // Limit the frames per second to a maximum value.
        // A value less than or equal to 0 means some default limit.
        // @see CamerasFramesBlender.cpp
        // @remarks Use this to reduce CPU usage.
        int maxOutputFps {20};

        bool operator==(const OutputPreviewConfiguration&) const = default;
    };
}  // namespace Observer
