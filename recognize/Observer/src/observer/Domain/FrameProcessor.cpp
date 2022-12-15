#include "FrameProcessor.hpp"

namespace Observer {

    FrameProcessor::FrameProcessor(const ProcessingConfiguration& cfg,
                                   double rotation)
        : lastFrame(cfg.resize, 3) {
        this->roi = cfg.roi;
        this->noiseThreshold = cfg.noiseThreshold;
        this->rotation = rotation;
        this->firstCall = true;
        this->resizeSize = cfg.resize;

        OBSERVER_ASSERT(this->roi.x + this->roi.width <= resizeSize.width &&
                            this->roi.y + this->roi.height <= resizeSize.height,
                        "The roi is taken from the resized image so it should "
                        "be inside it.");

        /* -------------- BUILD MASK FROM PARAMETER ------------- */
        this->mask = Frame(this->resizeSize, 1);  // starts as 0

        for (auto& maskPoints : cfg.masks) {
            ImageDraw::Get().FillConvexPoly(this->mask, maskPoints,
                                            ScalarVector::White());
        }

        this->mask.Resize(this->resizeSize);
    }

    FrameProcessor& FrameProcessor::NormalizeFrame(Frame& frame) & {
        frame.Resize(this->resizeSize);

        frame.Mask(this->mask);

        // rotation is an expensive operation, try to avoid it
        if (this->rotation != 0) {
            frame.RotateImage(rotation);
        }

        // crop the frame
        if (!this->roi.empty()) {
            frame.CropImage(roi);
        }

        // to black and white
        frame.ToColorSpace(ColorSpaceConversion::COLOR_RGB2GRAY);

        // set lastFrame to a valid frame so we can operate over it
        if (firstCall) {
            // ImageDisplay<T>::CreateWindow("ee");
            firstCall = false;
            frame.CopyTo(lastFrame);
        }

        // get the difference between the current and last frame
        diffFrame = frame.AbsoluteDifference(lastFrame);

        // ImageDisplay<T>::ShowImage("ee", this->diffFrame);

        // make the changes bigger
        diffFrame.GaussianBlur(3);

        // remove small changes/noise
        diffFrame.Threshold(noiseThreshold, 255, THRESHOLD_BINARY);

        frame.CopyTo(lastFrame);

        return *this;
    }

    double FrameProcessor::DetectChanges() { return diffFrame.CountNonZero(); }

}  // namespace Observer
