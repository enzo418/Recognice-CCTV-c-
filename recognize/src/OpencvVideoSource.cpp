#include "OpencvVideoSource.hpp"

namespace Observer {
    OpencvVideoSource::OpencvVideoSource() = default;

    void OpencvVideoSource::Open(const std::string& url) {
        this->videoCapture.open(url);
    }

    bool OpencvVideoSource::isOpened() { return this->videoCapture.isOpened(); }

    bool OpencvVideoSource::GetNextFrame(cv::Mat& frame) {
        return this->videoCapture.read(frame);
    }

    void OpencvVideoSource::Close(const std::string& url) {
        this->videoCapture.release();
    }

}  // namespace Observer
