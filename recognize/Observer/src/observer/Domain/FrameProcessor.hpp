#pragma once

#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Rect.hpp"

namespace Observer {

    class FrameProcessor {
       public:
        FrameProcessor() = default;

        FrameProcessor& NormalizeFrame(Frame& frame) &;

        double DetectChanges();

        /**
         * @brief This method should be called once connection to the camera was
         * established and before a call to NormalizeFrame/DetectChanges is
         * done.
         *
         * It's optimization because we prepare/calculate things that would
         * otherwise be calculated on every new frame.
         *
         * @param cameraFeedSize size of the frames received from the camera
         */
        void Setup(Size cameraFeedSize, const ProcessingConfiguration&,
                   double cameraRotation);

       private:
        // const Size resolutionSize = Size(640, 360);

        // last frame readed
        Frame lastFrame;

        // holder for the diff frame
        Frame diffFrame;

        // Calculated mask from the configuration passed
        Frame mask;

        //
        double accumulatorThresholds;

        Rect roi;

        double noiseThreshold;

        double rotation;

        bool firstCall;

        Size resizeSize;

        Size cameraFeedSize;

        std::vector<Mask> masks;
    };

}  // namespace Observer
