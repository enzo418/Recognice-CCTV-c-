#pragma once

#include <vector>

#include "../../Rect.hpp"

namespace Observer {
    struct ContoursFilter {
        struct IgnoredAreas {
            std::vector<Rect> areas;
            double minAreaPercentageToIgnore;

            bool operator==(const IgnoredAreas&) const = default;
        };

        // Each contour will need to have an area >= than the average area
        bool FilterByAverageArea;
        int MinimumArea;
        IgnoredAreas ignoredAreas;

        bool operator==(const ContoursFilter&) const = default;
    };
}  // namespace Observer