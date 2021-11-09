#include "RawCameraEvent.hpp"

namespace Observer {
    cv::Mat& RawCameraEvent::GetFrameAt(int index) & {
        return this->frames[index];
    }

    std::vector<cv::Mat>& RawCameraEvent::GetFrames() & { return this->frames; }

    std::vector<cv::Mat> RawCameraEvent::PopFrames() {
        return std::move(this->frames);
    }
}  // namespace Observer
