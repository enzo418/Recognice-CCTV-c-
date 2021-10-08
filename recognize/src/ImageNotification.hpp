#pragma once

#include "Notification.hpp"
#include "SpecialFunctions.hpp"

#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>

namespace Observer
{

    namespace fs = std::filesystem;

    class ImageNotification : public Notification
    {
    public:
        ImageNotification(int groupID, Event event, std::string text, cv::Mat& frame);

        std::string GetCaption();

        std::string GetImagePath();

        cv::Mat &GetImage();

        /**
         * @brief Build a image notification.
         * Once built, you can get the path where it is saved with `GetImagePath`
         * 
         * @param mediaFolderPath folder to save the image
         * @return true if could build and save the image
         * @return false 
         */
        bool BuildNotification(const std::string& mediaFolderPath);

    private:
        std::string text;

        // absolute path
        std::string outputImagePath;

        cv::Mat image;
    };

} // namespace Observer
