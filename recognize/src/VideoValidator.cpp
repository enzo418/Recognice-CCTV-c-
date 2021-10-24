#include "VideoValidator.hpp"

namespace Observer
{
    VideoValidator::VideoValidator(int sizeBufferBefore, int sizeBufferAfter) {
        this->framesBefore.emplace(sizeBufferBefore);
        this->framesAfter.emplace(sizeBufferAfter);

        this->changeDetected = false;
    }

    void VideoValidator::ChangeWasDetected() {
        this->changeDetected = true;
    }

    bool VideoValidator::AddFrame(cv::Mat &frame) {
        if (changeDetected) {
            return this->framesAfter->AddFrame(frame);
        } else {
            return this->framesBefore->AddFrame(frame);
        }
    }

    bool VideoValidator::CheckIfTheChangeIsValid() {
        // TODO
    }

    std::vector<cv::Mat> VideoValidator::GetFrames() {
        // TODO
    }
} // namespace Observer
