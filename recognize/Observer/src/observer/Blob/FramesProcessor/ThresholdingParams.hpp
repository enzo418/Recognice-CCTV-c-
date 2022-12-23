#pragma once

#include "observer/Size.hpp"

namespace Observer {
    struct ThresholdParams {
       public:
        struct ResizeParam {
            Size size {640, 360};
            bool resize {true};

            bool operator==(const ResizeParam&) const = default;
        };

       public:
        int FramesBetweenDiffFrames {3};
        int ContextFrames {2};
        int MedianBlurKernelSize {3};
        int GaussianBlurKernelSize {7};
        int DilationSize {2};

        // How much times brighter a pixels needs to be to be considered
        // useful, 5 is fine
        int BrightnessAboveThreshold {4};

        ResizeParam Resize;

        bool operator==(const ThresholdParams&) const = default;
    };
}  // namespace Observer