#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

#include "ImagePersistence.hpp"
#include "ImageTransformation.hpp"
#include "Notification.hpp"
#include "utils/SpecialFunctions.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    template <typename TFrame>
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
                          TFrame& frame);

        std::string GetCaption() override;

        std::string GetImagePath();

        TFrame& GetImage();

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

        TFrame image;
    };

    template <typename TFrame>
    ImageNotification<TFrame>::ImageNotification(int pGroupID, Event pEvent,
                                                 std::string pText,
                                                 TFrame& frame)
        : text(std::move(pText)), Notification(pGroupID, std::move(pEvent)) {
        ImageTransformation<TFrame>::CopyImage(frame, image);
    }

    template <typename TFrame>
    std::string ImageNotification<TFrame>::GetCaption() {
        return this->text;
    }

    template <typename TFrame>
    std::string ImageNotification<TFrame>::GetImagePath() {
        return this->outputImagePath;
    }

    template <typename TFrame>
    TFrame& ImageNotification<TFrame>::GetImage() {
        return this->image;
    }

    template <typename TFrame>
    bool ImageNotification<TFrame>::BuildNotification(
        const std::string& mediaFolderPath) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".jpg";
        const std::string& path =
            fs::path(mediaFolderPath) / fs::path(fileName);

        this->outputImagePath = path;

        return ImagePersistence<TFrame>::SaveImage(path, this->image);
    }
}  // namespace Observer
