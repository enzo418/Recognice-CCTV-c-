#pragma once

#include <string>
#include <opencv2/opencv.hpp>

namespace Observer
{
    class VideoWriter
    {
    public:
        VideoWriter() = default;

        virtual bool Open(const std::string &string, const double& framerate, const int& codecID, const cv::Size& frameSize) = 0;

        virtual void Close() = 0;

        virtual void WriteFrame(cv::Mat &frame) = 0;
    };

} // namespace Observer
