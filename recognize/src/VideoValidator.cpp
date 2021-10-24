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
        std::vector<cv::Mat> before = this->framesBefore->GetFrames();
        std::vector<cv::Mat> after = this->framesAfter->GetFrames();

        std::vector<cv::Mat> merged(before.size() + after.size());
        std::swap_ranges(merged.begin(), merged.end() - before.size(), before.begin());
        std::swap_ranges(merged.begin() + before.size(), merged.end(), after.begin());

        return merged;
    }
} // namespace Observer
