#pragma once

#include "../../VideoWriter.hpp"

namespace Observer {
    template <>
    class VideoWriter<cv::Mat> : public IVideoWriter<cv::Mat> {
       private:
        cv::VideoWriter writer;

       public:
        VideoWriter();

        bool Open(const std::string& path, const double& framerate,
                  const int& codecID, const cv::Size& frameSize) {
            return this->writer.open(path, codecID, framerate, frameSize);
        }

        void Close() { this->writer.release(); }

        void WriteFrame(cv::Mat& frame) { this->writer.write(frame); }
    };
}  // namespace Observer
