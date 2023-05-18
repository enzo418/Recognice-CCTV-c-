#pragma once

#include <string>
#include <vector>

namespace AsyncInference {
    struct SingleDetection {
        int x;
        int y;
        int width;
        int height;
        float confidence;
        std::string label;
    };

    struct ImageDetections {
        int image_index;
        std::vector<SingleDetection> detections;
    };
}  // namespace AsyncInference