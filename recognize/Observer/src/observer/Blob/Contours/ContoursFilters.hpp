#pragma once

#include <vector>

#include "observer/Rect.hpp"

namespace Observer {
    struct ContoursFilter {
        struct IgnoredSets {
            std::vector<std::vector<Point>> sets;

            // since the areas are points in some space, tell me its w/h.
            Size reference {640, 360};

            // [0-100] percentage of the number of vertices needed inside a set
            // to ignore a contour
            double minPercentageToIgnore {85};

            bool operator==(const IgnoredSets&) const = default;
        };

        // Each contour will need to have an area >= than the average area
        bool FilterByAverageArea {true};

        // in pixels, usually selected relative to the resize from threshold
        // parameters
        int MinimumArea {40};

        IgnoredSets ignoredSets;

        bool operator==(const ContoursFilter&) const = default;
    };
}  // namespace Observer