#pragma once

#include "../Implementation.hpp"
#include "../Log/log.hpp"
#include "../Rect.hpp"

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
