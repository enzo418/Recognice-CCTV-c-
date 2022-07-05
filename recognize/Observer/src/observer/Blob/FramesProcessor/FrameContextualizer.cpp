#include "FrameContextualizer.hpp"

namespace Observer {
    FrameContextualizer::FrameContextualizer(
        const ThresholdingParams& pThreshParams) {
        this->params = pThreshParams;

        this->contextDiffFrames.reserve(this->params.ContextFrames);

        this->InitializePostConstructor();
    }

    std::vector<Frame> FrameContextualizer::GenerateDiffFrames(
        std::vector<Frame>& frames) {
        std::vector<Frame> diffs(frames.size());
        std::vector<Frame> formatedFrames(frames.size());
        Frame diff;

        // 1. Get the diff between two frames
        for (int i = 1; i < frames.size(); i++) {
            // do not modify the frames that were given to us
            Frame current;
            Frame previous;

            frames[i].CopyTo(current);
            frames[i - 1].CopyTo(previous);

            if (params.Resize.resize) {
                current.Resize(params.Resize.size);
            }

            current.ToColorSpace(ColorSpaceConversion::COLOR_RGB2GRAY);

            formatedFrames[i] = current;

            if (i == 1) {
                if (params.Resize.resize) {
                    previous.Resize(params.Resize.size);
                }

                previous.ToColorSpace(ColorSpaceConversion::COLOR_RGB2GRAY);
                formatedFrames[0] = previous;
            }
        }

        // 2. Get the diff frames
        Frame lastFrame = formatedFrames[0];

        Size size = lastFrame.GetSize();
        // fill the firsts diff frames that we are going to skip...
        for (int w = 0; w < params.FramesBetweenDiffFrames; w++) {
            diffs[w] = lastFrame.GetBlackImage();
        }

        int diffSize = diffs.size();
        for (int i = params.FramesBetweenDiffFrames; i < frames.size();
             i += params.FramesBetweenDiffFrames) {
            // diff between the last 2 frames
            diff = lastFrame.AbsoluteDifference(formatedFrames[i]);

            this->ApplyThresholding(diff);

            // since there will not be diff frames in all the
            // FramesBetweenDiffFrames following frames, just use this one for
            // all of them
            for (int j = i;
                 j < std::min(i + params.FramesBetweenDiffFrames, diffSize);
                 j++) {
                diff.CopyTo(diffs[j]);
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
                diffs[initialPos].Add(diffs[initialPos - j]);
            }
        }

        return diffs;
    }

    Frame FrameContextualizer::GenerateDiffFrame(Frame& frame) {
        Frame grayFrame = frame.Clone();
        Frame diff;

        grayFrame.ToColorSpace(ColorSpaceConversion::COLOR_RGB2GRAY);

        if (frameCounter == 0) {
            grayFrame.CopyTo(this->lastFrame);
        }

        diff = grayFrame.AbsoluteDifference(this->lastFrame);

        // remove noise from the image
        this->ApplyThresholding(diff);

        // add the context
        diff.CopyTo(this->lastDiffFrameContext);
        this->AddContextToFrame(this->lastDiffFrameContext);

        // TODO: BENCHMARK THIS
        if (this->contextDiffFrames.size() >= this->params.ContextFrames) {
            this->contextDiffFrames.erase(this->contextDiffFrames.begin());
        }

        // add the image without the context
        this->contextDiffFrames.push_back(std::move(diff));

        // Every FramesBetweenDiffFrames update the last frame
        if (frameCounter % this->params.FramesBetweenDiffFrames == 0) {
            grayFrame.CopyTo(this->lastFrame);
        }

        frameCounter++;

        return this->lastDiffFrameContext;
    }

    void FrameContextualizer::AddContextToFrame(Frame& frame) {
        for (int k = 0; k < this->contextDiffFrames.size(); k++) {
            if (!this->contextDiffFrames[k].IsEmpty()) {
                frame.Add(this->contextDiffFrames[k]);
            }
        }
    }
}  // namespace Observer