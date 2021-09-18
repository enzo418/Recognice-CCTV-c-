#include "OpencvVideoSource.hpp"

namespace Observer
{
    OpencvVideoSource::OpencvVideoSource() { }

    void OpencvVideoSource::Open(const std::string& url) {
        this->capturer.open(url);
    }

    bool OpencvVideoSource::isOpened() {
        return this->capturer.isOpened();
    }

    bool OpencvVideoSource::GetNextFrame(cv::Mat& frame) {
        return this->capturer.read(frame);
    }

} // namespace Observer
