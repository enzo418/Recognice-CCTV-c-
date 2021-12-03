#pragma once

#include "../../VideoSource.hpp"

namespace Observer {
    template <>
    class VideoSource<cv::Mat> : public IVideoSource<cv::Mat> {
       public:
        VideoSource() = default;

        void Open(const std::string& url) override {
            this->videoCapture.open(url);
        }

        void Close(const std::string& url) override {
            this->videoCapture.release();
        }

        bool GetNextFrame(cv::Mat& frame) override {
            return this->videoCapture.read(frame);
        }

        bool isOpened() override { return this->videoCapture.isOpened(); }

       private:
        cv::VideoCapture videoCapture;
    };
}  // namespace Observer
