#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

#include "../../Implementation.hpp"
#include "../../Utils/SpecialFunctions.hpp"
#include "Notification.hpp"

namespace Observer {

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
                          Frame& frame);

        std::string GetCaption() override;

        std::string GetImagePath();

        Frame& GetImage();

        /**
         * @brief Build a image notification. Return the path
         *
         * @param mediaFolderPath folder to save the image
         * @return path
         */
        std::string BuildNotification(const std::string& mediaFolderPath);

        void Resize(const Size& target);
        void Resize(double fx, double fy);

       private:
        std::string text;

        // absolute path
        std::string outputImagePath;

        Frame image;
    };
}  // namespace Observer
