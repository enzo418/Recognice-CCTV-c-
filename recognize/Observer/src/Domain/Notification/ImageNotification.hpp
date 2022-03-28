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
        ImageNotification(int groupID, Event ev, Frame& frame,
                          std::string pOutputFolder);

        Frame& GetImage();

        /**
         * @brief Build a image notification. Return the path
         *
         * @param mediaFolderPath folder to save the image
         */
        void BuildNotification();

        void Resize(const Size& target);
        void Resize(double fx, double fy);

       private:
        Frame image;

       protected:
        static std::string CreatePath(const std::string& outputFolder);
    };
}  // namespace Observer
