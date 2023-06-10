#include "FrameProcessor.hpp"

#include <stdexcept>

namespace Observer {
    void FrameProcessor::Setup(Size pCameraFeedSize,
                               const ProcessingConfiguration& cfg,
                               double rotation) {
        this->lastFrame = Frame(cfg.resize, 3);
        this->roi = cfg.roi;
        this->noiseThreshold = cfg.noiseThreshold;
        this->firstCall = true;
        this->resizeSize = cfg.resize;
        this->masks = cfg.masks;
        this->rotation = rotation;

        this->cameraFeedSize = pCameraFeedSize;

        // if no resize was given use the camera resolution
        if (this->resizeSize.empty()) this->resizeSize = this->cameraFeedSize;

        /* -------------- BUILD MASK FROM PARAMETER ------------- */
        // use the input resolution because they were calculated with a raw
        // frame from it.
        this->mask = Frame(this->cameraFeedSize, 1);  // starts as 0

        for (auto& maskPoints : this->masks) {
            ImageDraw::Get().FillConvexPoly(this->mask, maskPoints,
                                            ScalarVector::White());
        }

        this->mask.Resize(this->resizeSize);

        /* ----------- RESIZE ROI TO MATCH LAST RESIZE ---------- */
        OBSERVER_ASSERT(
            this->roi.x + this->roi.width <= this->cameraFeedSize.width &&
                this->roi.y + this->roi.height <= this->cameraFeedSize.height,
            "The roi is relative to the input (camera) resolution so it should "
            "be included or be the same as it.");

        // Roi was selected relative to the camera resolution
        // We can't move the roi at the beginning of `NormalizeFrame` because we
        // don't can be sure that `frame` will always have the camera
        // resolution.
        double scaleX =
            this->resizeSize.width / (double)this->cameraFeedSize.width;
        double scaleY =
            this->resizeSize.height / (double)this->cameraFeedSize.height;

        this->roi.x *= scaleX;
        this->roi.width *= scaleX;
        this->roi.y *= scaleY;
        this->roi.height *= scaleY;
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
            // ImageDisplay::Get().CreateWindow("diff");
            // ImageDisplay::Get().CreateWindow("frame");

            if (this->cameraFeedSize.empty()) {
                throw std::logic_error("Precondition: call Setup before.");
            }

            firstCall = false;
            frame.CopyTo(lastFrame);
        }

        // ImageDisplay::Get().ShowImage("frame", frame);

        // get the difference between the current and last frame
        diffFrame = frame.AbsoluteDifference(lastFrame);

        // ImageDisplay::Get().ShowImage("diff", this->diffFrame);

        // make the changes bigger
        diffFrame.GaussianBlur(3);

        // remove small changes/noise
        diffFrame.Threshold(noiseThreshold, 255, THRESHOLD_BINARY);

        frame.CopyTo(lastFrame);

        return *this;
    }

    double FrameProcessor::DetectChanges() { return diffFrame.CountNonZero(); }

}  // namespace Observer
