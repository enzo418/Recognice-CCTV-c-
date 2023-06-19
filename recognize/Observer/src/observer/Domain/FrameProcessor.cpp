#include "FrameProcessor.hpp"

#include <stdexcept>

#include "observer/Log/log.hpp"

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

        if (pCameraFeedSize.empty()) {
            OBSERVER_WARN(
                "The camera feed size is empty, the frame processor will "
                "assume that the camera feed size is the same as the resize "
                "size.");

            this->cameraFeedSize = this->resizeSize;
        }

        // if no resize was given use the camera resolution
        if (this->resizeSize.empty()) {
            if (!this->cameraFeedSize.empty()) {
                this->resizeSize = this->cameraFeedSize;

            } else {
                OBSERVER_WARN(
                    "The resize size is empty, the frame processor will "
                    "assume that the resize size is 640x360.");

                // NOTE: Why set it to some value? Because this function caller
                // can come from the API or from the CameraObserver::Start.
                // If called from the API the camera feed might be not started,
                // and will be called again from the CameraObserver::Start. From
                // there, it's supposed to have a camera feed size, but if the
                // source size is 0x0, for some reason, we will have a problem
                // while calling openCV resize. Anyway masks doesn't matter if
                // the source is 0x0.

                this->resizeSize = Size(640, 360);
            }
        }

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
