#pragma once

#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Rect.hpp"

namespace Observer {

    class FrameProcessor {
       private:
        // const Size resolutionSize = Size(640, 360);

        // last frame readed
        Frame lastFrame;

        // holder for the diff frame
        Frame diffFrame;

        //
        double accumulatorThresholds;

        Rect roi;

        double noiseThreshold;

        double rotation;

        bool firstCall;

        Size resizeSize;

       public:
        /**
         * @brief Construct a new Frame Processor object
         *
         * @param resizeSize Even if the frame was already resize, user might
         * want to resize it again.
         * @param roi region of interest to crop
         * @param noiseThreshold
         * @param rotation angle to rotate the image in degrees
         */
        FrameProcessor(Size resizeSize, Rect roi, double noiseThreshold,
                       double rotation);

        FrameProcessor& NormalizeFrame(Frame& frame) &;

        double DetectChanges();
    };

}  // namespace Observer
