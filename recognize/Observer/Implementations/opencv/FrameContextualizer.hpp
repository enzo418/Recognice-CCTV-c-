#pragma once

#include <vector>

#include "../../src/BlobDetector/FramesProcessor/FrameContextualizer.hpp"

namespace Observer {
    template <>
    class FrameContextualizer<cv::Mat> : TFrameContextualizer<cv::Mat> {
       public:
        FrameContextualizer(const ThresholdingParams& params);

       public:
        std::vector<cv::Mat> GenerateDiffFrames(std::vector<cv::Mat>& frames);
        cv::Mat GenerateDiffFrame(cv::Mat& frame);

       public:
        long frameCounter {0};

       protected:
        void AddContextToFrame(cv::Mat& frame);

        void ApplyThresholding(cv::Mat& diffFrame) override {
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
                          mediumBrigthness * params.BrightnessAboveThreshold,
                          255, cv::THRESH_BINARY);
        }

        void InitializePostConstructor() override {
            morphologyElement = getStructuringElement(
                cv::MORPH_RECT,
                cv::Size(2 * params.DilationSize + 1,
                         2 * params.DilationSize + 1),
                cv::Point(params.DilationSize, params.DilationSize));
        }

       protected:
        cv::Mat morphologyElement;

        cv::Mat lastFrame;
        cv::Mat lastDiffFrameContext;

        std::vector<cv::Mat> contextDiffFrames;

       protected:
        ThresholdingParams params;
    };
}  // namespace Observer