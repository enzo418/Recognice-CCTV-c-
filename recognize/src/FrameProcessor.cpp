#include "FrameProcessor.hpp"

namespace Observer
{
    FrameProcessor::FrameProcessor(cv::Rect roi, double noiseThreshold) {
        this->roi = roi;
        this->noiseThreshold = noiseThreshold;
    }

    FrameProcessor& FrameProcessor::NormalizeFrame(cv::Mat& frame) & {
        cv::resize(frame, frame, this->resolutionSize);

        // TODO: RotateImage(this->frame, this->config->rotation);

        // crop the frame
        if (!this->roi.empty()) {
		    frame = frame(this->roi);
	    }

        // to black and white
        cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

        // get the difference between the current and last frame
        cv::absdiff(this->lastFrame, frame, this->diffFrame);

        // make the changes bigger 
        cv::GaussianBlur(this->diffFrame, this->diffFrame, cv::Size(3, 3), 10);

        // remove small changes
        cv::threshold(this->diffFrame, this->diffFrame, this->noiseThreshold, 255, cv::THRESH_BINARY);

        frame.copyTo(this->lastFrame);
    }

    double FrameProcessor::DetectChanges() {
        return cv::countNonZero(this->diffFrame);
    }
} // namespace Observer
