#pragma once

#include <vector>

#include "ThresholdingParams.hpp"
#include "observer/Implementation.hpp"

namespace Observer {

    /**
     * @brief Generates a frame with all the context requested in the parameters
     * based on previous frames.
     *
     * it's a incomplete templated class, that means a especialization is needed
     * for some of its methods.
     */
    class FrameContextualizer {
       public:
        FrameContextualizer(const ThresholdParams& params);

       public:
        std::vector<Frame> GenerateDiffFrames(std::vector<Frame>& frames);
        Frame GenerateDiffFrame(Frame& frame);

       public:
        long frameCounter {0};

       protected:
        void AddContextToFrame(Frame& frame);

        /**
         * @brief Apply threshold to a image.
         * Each especialization it should implement a thresholding based on the
         * parameters given.
         *
         * @param frame
         */
        void ApplyThreshold(Frame& frame);

        /**
         * @brief Initialize members like morphologyElement to optimize memory
         * usage.
         */
        void InitializePostConstructor();

       protected:
        Frame::IType morphologyElement;

        Frame lastFrame;
        Frame lastDiffFrameContext;

        std::vector<Frame> contextDiffFrames;

       protected:
        ThresholdParams params;
    };
}  // namespace Observer