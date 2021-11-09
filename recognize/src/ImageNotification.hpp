#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

#include "Notification.hpp"
#include "utils/SpecialFunctions.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    class ImageNotification : public Notification {
       public:
        /**
         * Creates a image notification.
         * Don't worry deleting the frame after creating this object since
         * we copy it.
         *
         * @param groupID
         * @param text caption
         * @param frame frame, as reference but copied on ctor.
         */
        ImageNotification(int groupID, Event ev, std::string text,
                          cv::Mat& frame);

        std::string GetCaption() override;

        std::string GetImagePath();

        cv::Mat& GetImage();

        /**
         * @brief Build a image notification.
         * Once built, you can get the path where it is saved with
         * `GetImagePath`
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

}  // namespace Observer
