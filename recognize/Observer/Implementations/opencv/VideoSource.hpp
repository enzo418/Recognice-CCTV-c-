#pragma once

#include "../../src/Domain/VideoSource.hpp"

namespace Observer {
    template <>
    class VideoSource<cv::Mat> : public IVideoSource<cv::Mat> {
       public:
        VideoSource() = default;

        void Open(const std::string& url) override {
            this->videoCapture.open(url);
        }

        void Close() override { this->videoCapture.release(); }

        bool GetNextFrame(cv::Mat& frame) override {
            return this->videoCapture.read(frame);
        }

        bool isOpened() override { return this->videoCapture.isOpened(); }

        int GetFPS() override { return videoCapture.get(cv::CAP_PROP_FPS); }

       private:
        cv::VideoCapture videoCapture;
    };
}  // namespace Observer
