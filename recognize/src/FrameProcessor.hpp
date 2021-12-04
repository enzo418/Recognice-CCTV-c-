#pragma once

#include <opencv2/opencv.hpp>

#include "ImageTransformation.hpp"
#include "Rect.hpp"

namespace Observer {
    template <typename T>
    class FrameProcessor {
       private:
        const Size resolutionSize = Size(640, 360);

        // last frame readed
        T lastFrame;

        // holder for the diff frame
        T diffFrame;

        //
        double accumulatorThresholds;

        Rect roi;

        double noiseThreshold;

        double rotation;

       public:
        /**
         * @brief Construct a new Frame Processor object
         *
         * @param roi region of interest to crop
         * @param noiseThreshold
         * @param rotation angle to rotate the image in degrees
         */
        FrameProcessor(Rect roi, double noiseThreshold, double rotation);

        FrameProcessor& NormalizeFrame(T& frame) &;

        double DetectChanges();
    };

    template <typename T>
    FrameProcessor<T>::FrameProcessor(Rect pRoi, double pNoiseThreshold,
                                      double pRotation) {
        this->roi = pRoi;
        this->noiseThreshold = pNoiseThreshold;
        this->rotation = pRotation;
        this->lastFrame = ImageTransformation<T>::BlackImage();
    }

    template <typename T>
    FrameProcessor<T>& FrameProcessor<T>::NormalizeFrame(T& frame) & {
        ImageTransformation<T>::Resize(frame, frame, this->resolutionSize);

        ImageTransformation<T>::RotateImage(frame, this->rotation);

        // crop the frame
        if (!this->roi.empty()) {
            ImageTransformation<T>::CropImage(frame, frame, this->roi);
        }

        // to black and white
        ImageTransformation<T>::ToColorSpace(
            frame, frame, ColorSpaceConversion::COLOR_RGB2GRAY);

        // TODO: PROBLABLY HERE IS MISMATCH OF SIZES BETWEEN lastframe and
        // frame, since lastframe in the first iteration is empty (BlackImage)        

        // get the difference between the current and last frame
        ImageTransformation<T>::AbsoluteDifference(this->lastFrame, frame,
                                                   this->diffFrame);

        // make the changes bigger
        ImageTransformation<T>::GaussianBlur(this->diffFrame, this->diffFrame,
                                             3);

        // remove small changes/noise
        ImageTransformation<T>::Threshold(this->diffFrame, this->diffFrame,
                                          this->noiseThreshold, 255,
                                          THRESHOLD_BINARY);

        ImageTransformation<T>::CopyImage(frame, lastFrame);

        return *this;
    }

    template <typename T>
    double FrameProcessor<T>::DetectChanges() {
        return ImageTransformation<T>::CountNonZero(this->diffFrame);
    }

}  // namespace Observer
