#pragma once

#include "VideoSource.hpp"

namespace Observer {
    class OpencvVideoSource : public VideoSource {
       public:
        OpencvVideoSource();

        void Open(const std::string& url) override;

        void Close(const std::string& url) override;

        bool GetNextFrame(cv::Mat&) override;

        bool isOpened() override;

       private:
        cv::VideoCapture videoCapture;
    };

}  // namespace Observer
