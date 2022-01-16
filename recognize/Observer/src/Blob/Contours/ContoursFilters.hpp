#pragma once

#include <vector>

#include "../../Rect.hpp"

namespace Observer {
    struct ContoursFilter {
        struct IgnoredAreas {
            std::vector<Rect> areas;
            double minAreaPercentageToIgnore;

            // since the areas are points in some space, tell me its w/h.
            Size reference;

            bool operator==(const IgnoredAreas&) const = default;
        };

        struct IgnoredSets {
            std::vector<std::vector<Point>> sets;

            // since the areas are points in some space, tell me its w/h.
            Size reference;

            bool operator==(const IgnoredSets&) const = default;
        };

        // Each contour will need to have an area >= than the average area
        bool FilterByAverageArea;
        int MinimumArea;
        IgnoredAreas ignoredAreas;
        IgnoredSets ignoredSets;

        bool operator==(const ContoursFilter&) const = default;
    };
}  // namespace Observer