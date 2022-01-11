#pragma once

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

#include "../../ImagePersistence.hpp"
#include "../../ImageTransformation.hpp"
#include "../../Utils/SpecialFunctions.hpp"
#include "Notification.hpp"

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
    std::string ImageNotification<TFrame>::BuildNotification(
        const std::string& mediaFolderPath) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".jpg";
        const std::string& path =
            fs::path(mediaFolderPath) / fs::path(fileName);

        ImagePersistence<TFrame>::SaveImage(path, this->image);

        return path;
    }

    template <typename TFrame>
    void ImageNotification<TFrame>::Resize(const Size& target) {
        ImageTransformation<TFrame>::Resize(image, image, target);
    }

    template <typename TFrame>
    void ImageNotification<TFrame>::Resize(double fx, double fy) {
        Size size = ImageTransformation<TFrame>::GetSize(image);
        size.width *= fx;
        size.height *= fy;

        this->Resize(size);
    }
}  // namespace Observer
