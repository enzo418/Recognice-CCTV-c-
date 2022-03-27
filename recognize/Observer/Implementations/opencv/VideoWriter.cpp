#include "VideoWriter.hpp"

namespace Observer {
    bool VideoWriter::Open(const std::string& path, const double& framerate,
                           const int& codecID, const Size& frameSize) {
        cv::Size sz(frameSize.width, frameSize.height);
        return this->writer.open(path, codecID, framerate, sz);
    }

    void VideoWriter::Close() { this->writer.release(); }

    void VideoWriter::WriteFrame(Frame& frame) {
        this->writer.write(frame.GetInternalFrame());
    }

    int VideoWriter::GetDefaultCodec() {
        return cv::VideoWriter::fourcc('H', '2', '6', '4');
    }
}  // namespace Observer
