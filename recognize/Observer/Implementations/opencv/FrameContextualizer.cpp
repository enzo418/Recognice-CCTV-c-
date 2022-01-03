

#include "../../src/Blob/FramesProcessor/FrameContextualizer.hpp"

#include <vector>

namespace Observer {
    template <>
    void FrameContextualizer<cv::Mat>::InitializePostConstructor() {
        morphologyElement = getStructuringElement(
            cv::MORPH_RECT,
            cv::Size(2 * params.DilationSize + 1, 2 * params.DilationSize + 1),
            cv::Point(params.DilationSize, params.DilationSize));
    }

    template <>
    void FrameContextualizer<cv::Mat>::ApplyThresholding(cv::Mat& diffFrame) {
        // Normalizing noise
        cv::medianBlur(diffFrame, diffFrame, params.MedianBlurKernelSize);

        // Normalizing noise 2
        cv::GaussianBlur(diffFrame, diffFrame,
                         cv::Size(params.GaussianBlurKernelSize,
                                  params.GaussianBlurKernelSize),
                         10);

        // Normalizing noise 3
        cv::morphologyEx(diffFrame, diffFrame, cv::MORPH_OPEN,
                         morphologyElement);

        double mediumBrigthness = 10;
        auto nonzero = cv::countNonZero(diffFrame);
        if (nonzero != 0) {
            mediumBrigthness = (double)cv::sum(diffFrame)[0] / nonzero;
        }
        cv::threshold(diffFrame, diffFrame,
                      mediumBrigthness * params.BrightnessAboveThreshold, 255,
                      cv::THRESH_BINARY);
    }
}  // namespace Observer