#pragma once

namespace Observer {
    struct BlobDetectorParams {
        // Maximum distance between a blob and a finding to be considered the
        // same
        int distance_thresh {80};

        // How similar a finding and a blob needs to be to be considered the
        // same
        double similarity_threshold {0.6};

        // Initial life of a blob, in number of frames
        int blob_max_life {14};

        bool operator==(const BlobDetectorParams&) const = default;
    };
}  // namespace Observer