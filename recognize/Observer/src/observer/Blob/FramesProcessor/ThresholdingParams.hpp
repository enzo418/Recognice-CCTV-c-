#pragma once

#include "observer/Size.hpp"

namespace Observer {
    struct ThresholdParams {
       public:
        struct ResizeParam {
            Size size;
            bool resize;

            bool operator==(const ResizeParam&) const = default;
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

        bool operator==(const ThresholdParams&) const = default;
    };
}  // namespace Observer