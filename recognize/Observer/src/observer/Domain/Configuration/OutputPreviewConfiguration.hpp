#pragma once

#include "observer/Size.hpp"

namespace Observer {
    struct OutputPreviewConfiguration {
        bool showOutput {true};
        Size resolution {640, 360};
        double scaleFactor {1};

        bool operator==(const OutputPreviewConfiguration&) const = default;
    };
}  // namespace Observer
