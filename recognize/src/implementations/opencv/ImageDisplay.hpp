#pragma once

#include <iostream>
#include <opencv2/highgui.hpp>
#include "opencv2/opencv.hpp"

#include "../../ImageDisplay.hpp"

namespace Observer {
    template <>
    struct ImageDisplay<cv::Mat> {
        static void CreateWindow(const std::string& name) {
            cv::namedWindow(name, cv::WINDOW_AUTOSIZE);
        }

        static inline void ShowImage(const std::string& windowName, cv::Mat& image) {
            cv::imshow(windowName, image);
        }

        static void DestroyWindow(const std::string& name) {
            cv::destroyWindow(name);
        }
    };
}  // namespace Observer