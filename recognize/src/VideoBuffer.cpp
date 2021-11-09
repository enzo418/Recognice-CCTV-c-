#include "VideoBuffer.hpp"

namespace Observer {
    VideoBuffer::VideoBuffer(int sizeBufferBefore, int sizeBufferAfter) {
        this->framesBefore.emplace(sizeBufferBefore);
        this->framesAfter.emplace(sizeBufferAfter);

        this->changeDetected = false;
    }

    void VideoBuffer::ChangeWasDetected() { this->changeDetected = true; }

    bool VideoBuffer::AddFrame(cv::Mat& frame) {
        if (changeDetected) {
            return this->framesAfter->AddFrame(frame);
        } else {
            return this->framesBefore->AddFrame(frame);
        }
    }

    bool VideoBuffer::CheckIfTheChangeIsValid() {
        // TODO
        return true;
    }

    RawCameraEvent VideoBuffer::GetEventFound() {
        std::vector<cv::Mat> before = this->framesBefore->GetFrames();
        std::vector<cv::Mat> after = this->framesAfter->GetFrames();
        int firstFrameWhereChangeWasFound = before.size() - 1;

        std::vector<cv::Mat> merged(before.size() + after.size());
        std::swap_ranges(merged.begin(), merged.end() - before.size(),
                         before.begin());
        std::swap_ranges(merged.begin() + before.size(), merged.end(),
                         after.begin());

        RawCameraEvent ev(std::move(merged), firstFrameWhereChangeWasFound);

        return ev;
    }
}  // namespace Observer
