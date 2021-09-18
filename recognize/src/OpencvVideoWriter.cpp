#include "OpencvVideoWriter.hpp"

namespace Observer
{
    OpencvVideoWritter::OpencvVideoWritter(){};

    bool OpencvVideoWritter::Open(const std::string &path, const double& framerate, const int& codecID, const cv::Size& frameSize){
        this->writer.open(path, codecID, framerate, frameSize);
    }

    void OpencvVideoWritter::Close() {
        this->writer.release();
    }

    void OpencvVideoWritter::WriteFrame(cv::Mat& frame) {
        this->writer.write(frame);
    }

} // namespace Observer
