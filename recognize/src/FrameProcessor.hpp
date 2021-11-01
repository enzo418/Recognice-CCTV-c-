#pragma once

#include <opencv2/opencv.hpp>

namespace Observer
{
    class FrameProcessor
    {
    private:
        const cv::Size resolutionSize = cv::Size(640, 360);

        // last frame readed
        cv::Mat lastFrame;

        // holder for the diff frame
        cv::Mat diffFrame;

        // 
        double accumulatorThresholds;

        cv::Rect roi;
        
        double noiseThreshold;

    public:
        FrameProcessor(cv::Rect roi, double noiseThreshold);

        FrameProcessor& NormalizeFrame(cv::Mat &frame) &;

        double DetectChanges();
    };
    
} // namespace Observer
