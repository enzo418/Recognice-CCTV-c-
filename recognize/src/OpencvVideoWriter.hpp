#pragma once

#include "VideoWriter.hpp"

namespace Observer
{
    class OpencvVideoWritter :  public VideoWriter
    {
    private:
        cv::VideoWriter writer;

    public:
        OpencvVideoWritter();

        bool Open(const std::string &path, const double& framerate, const int& codecID, const cv::Size& frameSize);

        void Close();

        void WriteFrame(cv::Mat &frame);
    };
    
} // namespace Observer
