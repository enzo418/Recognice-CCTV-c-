#pragma once

#include <string>

#include <opencv2/opencv.hpp>


namespace Observer
{
    class VideoSource {
        public:
            virtual void Open(const std::string& url) = 0;

            virtual void Close(const std::string& url) = 0;

            virtual bool GetNextFrame(cv::Mat&) = 0;

            virtual bool isOpened() = 0;
    };
} // namespace Observer
