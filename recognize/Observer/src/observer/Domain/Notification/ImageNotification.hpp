#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

#include "Notification.hpp"
#include "observer/Implementation.hpp"
#include "observer/Utils/SpecialFunctions.hpp"

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
        ImageNotification(int groupID, EventDescriptor ev, Frame& frame,
                          const std::string& pOutputFolder,
                          std::shared_ptr<std::vector<object_detected_t>>&
                              simplifiedObjectsDetected);

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
