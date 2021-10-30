#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

/**
 * @brief Holds all the frames and the
 * first frame where a change was found.
 */
struct RawCameraEvent {
    public:
        RawCameraEvent() {}

        // && -> pFrames can only be r-value (std::move is necessary)
        RawCameraEvent(std::vector<cv::Mat>&& pFrames, int pIndexFirst)
        :   frames(std::move(pFrames)),
            frameIndexOfFirstChange(pIndexFirst) { }
    public:
        std::vector<cv::Mat> frames;
        int frameIndexOfFirstChange;
};
