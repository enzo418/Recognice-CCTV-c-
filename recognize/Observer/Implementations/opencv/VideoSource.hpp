#pragma once

#include "../../src/Domain/VideoSource.hpp"

namespace Observer {
    template <>
    class VideoSource<cv::Mat> : public IVideoSource<cv::Mat> {
       public:
        VideoSource() = default;

        void Open(const std::string& url) override {
            try {
                this->videoCapture.open(url);
            } catch (...) {
                // ffmpeg if cannot open it terminates the program, so we need
                // to catch it. To know if the source is ok check for isOpened
            }
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
