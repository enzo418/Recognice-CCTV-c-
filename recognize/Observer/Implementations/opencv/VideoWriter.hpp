#pragma once

#include "../../src/Domain/VideoWriter.hpp"

namespace Observer {
    template <>
    class VideoWriter<cv::Mat> : public IVideoWriter<cv::Mat> {
       private:
        cv::VideoWriter writer;

       public:
        VideoWriter() = default;

        bool Open(const std::string& path, const double& framerate,
                  const int& codecID, const Size& frameSize) override {
            cv::Size sz(frameSize.width, frameSize.height);
            return this->writer.open(path, codecID, framerate, sz);
        }

        void Close() override { this->writer.release(); }

        void WriteFrame(cv::Mat& frame) override { this->writer.write(frame); }

        int GetDefaultCodec() override {
            return cv::VideoWriter::fourcc('H', '2', '6', '4');
        }
    };
}  // namespace Observer
