

#include "observer/Blob/FramesProcessor/FrameContextualizer.hpp"

namespace Observer {
    void FrameContextualizer::InitializePostConstructor() {
        morphologyElement = getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(2 * params.DilationSize + 1, 2 * params.DilationSize + 1),
            cv::Point(params.DilationSize, params.DilationSize));
    }

    void FrameContextualizer::ApplyThreshold(Frame& diffFrame) {
        auto& frame = diffFrame.GetInternalFrame();

        // Normalizing noise
        cv::medianBlur(frame, frame, params.MedianBlurKernelSize);

        // Normalizing noise 2
        cv::GaussianBlur(frame, frame,
                         cv::Size(params.GaussianBlurKernelSize,
                                  params.GaussianBlurKernelSize),
                         10);

        // Normalizing noise 3
        cv::morphologyEx(frame, frame, cv::MORPH_OPEN, morphologyElement);

        double mediumBrightness = 10;
        auto nonzero = cv::countNonZero(frame);
        if (nonzero != 0) {
            mediumBrightness = (double)cv::sum(frame)[0] / nonzero;
        }
        cv::threshold(frame, frame,
                      mediumBrightness * params.BrightnessAboveThreshold, 255,
                      cv::THRESH_BINARY);
    }
}  // namespace Observer