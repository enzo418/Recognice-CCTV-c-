#pragma once

#include <vector>

#include "../../ImageTransformation.hpp"
#include "ThresholdingParams.hpp"

namespace Observer {

    /**
     * @brief Generates a frame with all the context requested in the parameters
     * based on previous frames.
     *
     * T indicates it's a incomplete templated class, that means a
     * especialization is needed for some of its methods.
     */
    template <typename TFrame>
    class TFrameContextualizer {
       public:
        TFrameContextualizer(const ThresholdingParams& params);

       public:
        std::vector<TFrame> GenerateDiffFrames(std::vector<TFrame>& frames);
        TFrame GenerateDiffFrame(TFrame& frame);

       public:
        long frameCounter {0};

       protected:
        void AddContextToFrame(TFrame& frame);

        /**
         * @brief Apply threshold to a image.
         * Each especialization it should implement a thresholding based on the
         * parameters given.
         *
         * @param frame
         */
        virtual void ApplyThresholding(TFrame& frame) = 0;

        /**
         * @brief Initialize members like morphologyElement to optimize memory
         * usage.
         */
        virtual void InitializePostConstructor() = 0;

       protected:
        TFrame morphologyElement;

        TFrame lastFrame;
        TFrame lastDiffFrameContext;

        std::vector<TFrame> contextDiffFrames;

       protected:
        ThresholdingParams params;
    };

    // Forward declaration.
    // https://google.github.io/styleguide/cppguide.html#Forward_Declarations
    template <typename T>
    class FrameContextualizer : public TFrameContextualizer<T> {};
}  // namespace Observer