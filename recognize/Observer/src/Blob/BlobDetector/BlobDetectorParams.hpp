#pragma once

struct BlobDetectorParams {
    // Maximum distance between a blob and a finding to be considered the same
    int distance_thresh;

    // How similar a finding and a blob needs to be to be considered the same
    double similarity_threshold;

    // Initial life of a blob, in number of frames
    int blob_max_life;
};