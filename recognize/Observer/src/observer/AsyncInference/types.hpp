#pragma once

#include <string>
#include <vector>

namespace AsyncInference {
    struct SingleDetection {
        float x;
        float y;
        float width;
        float height;
        float confidence;
        std::string label;
    };

    struct ImageDetections {
        int image_index;
        std::vector<SingleDetection> detections;
    };
}  // namespace AsyncInference