#include "FrameProcessor.hpp"

namespace Observer {

    FrameProcessor::FrameProcessor(Size pResizeSize, Rect pRoi,
                                   double pNoiseThreshold, double pRotation)
        : lastFrame(pResizeSize, 3) {
        this->roi = pRoi;
        this->noiseThreshold = pNoiseThreshold;
        this->rotation = pRotation;
        this->firstCall = true;
        this->resizeSize = pResizeSize;

        OBSERVER_ASSERT(pRoi.x + pRoi.width <= resizeSize.width &&
                            pRoi.y + pRoi.height <= resizeSize.height,
                        "The roi is taken from the resized image so it should "
                        "be inside it.");
    }

    FrameProcessor& FrameProcessor::NormalizeFrame(Frame& frame) & {
        frame.Resize(this->resizeSize);

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
