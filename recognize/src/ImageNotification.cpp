#include "ImageNotification.hpp"

#include <utility>

namespace Observer
{
    ImageNotification::ImageNotification(
            int pGroupID,
            Event pEvent,
            std::string pText,
            cv::Mat& frame)
        : text(std::move(pText)), Notification(pGroupID, std::move(pEvent)) {
        frame.copyTo(image);
    }

    std::string ImageNotification::GetCaption() {
        return this->text;
    }

    std::string ImageNotification::GetImagePath() {
        return this->outputImagePath;
    }

    cv::Mat &ImageNotification::GetImage() {
        return this->image;
    }

    bool ImageNotification::BuildNotification(const std::string& mediaFolderPath) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".jpg";
        const std::string &path = fs::path(mediaFolderPath) / fs::path(fileName);

        this->outputImagePath = path;

        return cv::imwrite(path, this->image);
    }
} // namespace Observer
