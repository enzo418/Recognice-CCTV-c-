#pragma once

namespace Observer {
    struct BlobFilters {
       public:
        struct BlobVelocityFilters {
            bool UseVelocityFilter;

            double MinVelocity;
            double MaxVelocity;

            bool operator==(const BlobVelocityFilters&) const = default;
        };

       public:
        // E.g. delete blob if it only appears in only 1 frame
        int MinimumOccurrences {14};

        // unit = 1% of the diagonal
        int MinimumUnitsTraveled {0};

        BlobVelocityFilters VelocityFilter {
            .UseVelocityFilter = false, .MinVelocity = 0, .MaxVelocity = 0};

        bool operator==(const BlobFilters&) const = default;
    };
}  // namespace Observer