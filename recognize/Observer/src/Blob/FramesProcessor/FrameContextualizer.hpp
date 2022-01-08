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
    class FrameContextualizer {
       public:
        FrameContextualizer(const ThresholdingParams& params);

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
        void ApplyThresholding(TFrame& frame);

        /**
         * @brief Initialize members like morphologyElement to optimize memory
         * usage.
         */
        void InitializePostConstructor();

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
    class FrameContextualizer;

    template <typename TFrame>
    FrameContextualizer<TFrame>::FrameContextualizer(
        const ThresholdingParams& pThreshParams) {
        this->params = pThreshParams;

        this->contextDiffFrames.reserve(this->params.ContextFrames);

        this->InitializePostConstructor();
    }

    template <typename TFrame>
    std::vector<TFrame> FrameContextualizer<TFrame>::GenerateDiffFrames(
        std::vector<TFrame>& frames) {
        std::vector<TFrame> diffs(frames.size());
        std::vector<TFrame> formatedFrames(frames.size());
        TFrame diff;

        // 1. Get the diff between two frames
        for (int i = 1; i < frames.size(); i++) {
            // do not modify the frames that were given to us
            TFrame current;
            TFrame previous;

            ImageTransformation<TFrame>::CopyImage(frames[i], current);
            ImageTransformation<TFrame>::CopyImage(frames[i - 1], previous);

            if (params.Resize.resize) {
                ImageTransformation<TFrame>::Resize(current, current,
                                                    params.Resize.size);
            }

            ImageTransformation<TFrame>::ToColorSpace(
                current, current, ColorSpaceConversion::COLOR_RGB2GRAY);

            formatedFrames[i] = current;

            if (i == 1) {
                if (params.Resize.resize) {
                    ImageTransformation<TFrame>::Resize(previous, previous,
                                                        params.Resize.size);
                }

                ImageTransformation<TFrame>::ToColorSpace(
                    previous, previous, ColorSpaceConversion::COLOR_RGB2GRAY);
                formatedFrames[0] = previous;
            }
        }

        // 2. Get the diff frames
        TFrame lastFrame = formatedFrames[0];

        // fill the firsts diff frames that we are going to skip...
        for (int w = 0; w < params.FramesBetweenDiffFrames; w++) {
            diffs[w] =
                ImageTransformation<TFrame>::BlackImage(params.Resize.size, 1);
        }

        int diffSize = diffs.size();
        for (int i = params.FramesBetweenDiffFrames; i < frames.size();
             i += params.FramesBetweenDiffFrames) {
            // diff between the last 2 frames
            ImageTransformation<TFrame>::AbsoluteDifference(
                lastFrame, formatedFrames[i], diff);

            this->ApplyThresholding(diff);

            // since there will not be diff frames in all the
            // FramesBetweenDiffFrames following frames, just use this one for
            // all of them
            for (int j = i;
                 j < std::min(i + params.FramesBetweenDiffFrames, diffSize);
                 j++) {
                ImageTransformation<TFrame>::CopyImage(diff, diffs[j]);
            }

            lastFrame = formatedFrames[i];
        }

        diffs[0] = diffs[1];

        // 2. Adds n diff frames
        // The reason behind this is to add context to all the diffs frames.
        // This means that a diff frame at the right of some frames will have
        // the context (movement) of the previous frames, this giving a more
        // accurate reading
        for (int initialPos = diffs.size() - 1;
             initialPos >= params.ContextFrames; initialPos--) {
            // This could be optimized this by adding in group of two
            for (int j = params.ContextFrames; j > 0; j--) {
                ImageTransformation<TFrame>::AddImages(diffs[initialPos - j],
                                                       diffs[initialPos],
                                                       diffs[initialPos]);
            }
        }

        return diffs;
    }

    template <typename TFrame>
    TFrame FrameContextualizer<TFrame>::GenerateDiffFrame(TFrame& frame) {
        TFrame grayFrame;
        TFrame diff;
        ImageTransformation<TFrame>::ToColorSpace(
            frame, grayFrame, ColorSpaceConversion::COLOR_RGB2GRAY);

        if (frameCounter == 0) {
            ImageTransformation<TFrame>::CopyImage(grayFrame, this->lastFrame);
        }

        ImageTransformation<TFrame>::AbsoluteDifference(grayFrame,
                                                        this->lastFrame, diff);

        // remove noise from the image
        this->ApplyThresholding(diff);

        // add the context
        ImageTransformation<TFrame>::CopyImage(diff,
                                               this->lastDiffFrameContext);
        this->AddContextToFrame(this->lastDiffFrameContext);

        // TODO: BENCHMARK THIS
        if (this->contextDiffFrames.size() >= this->params.ContextFrames) {
            this->contextDiffFrames.erase(this->contextDiffFrames.begin());
        }

        // add the image without the context
        this->contextDiffFrames.push_back(std::move(diff));

        // Every FramesBetweenDiffFrames update the last frame
        if (frameCounter % this->params.FramesBetweenDiffFrames == 0) {
            ImageTransformation<TFrame>::CopyImage(grayFrame, this->lastFrame);
        }

        frameCounter++;

        return this->lastDiffFrameContext;
    }

    template <typename TFrame>
    void FrameContextualizer<TFrame>::AddContextToFrame(TFrame& frame) {
        for (int k = 0; k < this->contextDiffFrames.size(); k++) {
            if (!this->contextDiffFrames[k].empty()) {
                ImageTransformation<TFrame>::AddImages(
                    frame, this->contextDiffFrames[k], frame);
            }
        }
    }
}  // namespace Observer