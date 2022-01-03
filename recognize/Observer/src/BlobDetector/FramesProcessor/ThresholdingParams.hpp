#pragma once

#include "../../Size.hpp"

namespace Observer {
    struct ThresholdingParams {
       public:
        struct ResizeParam {
            Size size;
            bool resize;
        };

       public:
        int FramesBetweenDiffFrames;
        int ContextFrames;
        int MedianBlurKernelSize;
        int GaussianBlurKernelSize;
        int DilationSize;

        // How much times brighter a pixels needs to be to be considered
        // usefull, 5 is fine
        int BrightnessAboveThreshold;
        ResizeParam Resize;
    };
}  // namespace Observer