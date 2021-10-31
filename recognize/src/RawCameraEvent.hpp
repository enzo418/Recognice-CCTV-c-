#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

namespace Observer {
    /**
     * @brief Holds all the frames and the
     * first frame where a change was found.
     */
    struct RawCameraEvent {
    public:
        RawCameraEvent() = default;

        // && -> pFrames can only be r-value (std::move is necessary)
        RawCameraEvent(std::vector<cv::Mat> &&pFrames, int pIndexFirst)
                : frames(std::move(pFrames)),
                  frameIndexOfFirstChange(pIndexFirst) {}

        cv::Mat &GetFrameAt(int index) &;

        std::vector<cv::Mat> &GetFrames() &;

        /**
         * Returns a move instructor of the frames,
         * this object should be deleted after this call.
         * @return rvalue - frames
         */
        std::vector<cv::Mat> PopFrames();

    private:
        std::vector<cv::Mat> frames;
        int frameIndexOfFirstChange{};
    };
}