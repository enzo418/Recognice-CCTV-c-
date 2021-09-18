#pragma once

#include "VideoSource.hpp"

namespace Observer
{
    class OpencvVideoSource : public VideoSource
    {
        private:
            cv::VideoCapture capturer;

        public:
            OpencvVideoSource();

            void Open(const std::string& url);

            void Close(const std::string& url);

            bool GetNextFrame(cv::Mat&);

            bool isOpened();
    };
    
} // namespace Observer
