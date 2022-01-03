#pragma once

struct BlobFilters {
   public:
    struct BlobVelocityFilters {
        bool UseVelocityFilter;

        double MinVelocity;
        double MaxVelocity;
    };

   public:
    // E.g. delete blob if it only appears in only 1 frame
    int MinimumOccurrences;

    BlobVelocityFilters VelocityFilter;
};