#include "ImageNotification.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    ImageNotification::ImageNotification(int pGroupID, Event pEvent,
                                         std::string pText, Frame& frame)
        : text(std::move(pText)), Notification(pGroupID, std::move(pEvent)) {
        frame.CopyTo(image);
    }

    std::string ImageNotification::GetCaption() { return this->text; }

    std::string ImageNotification::GetImagePath() {
        return this->outputImagePath;
    }

    Frame& ImageNotification::GetImage() { return this->image; }

    std::string ImageNotification::BuildNotification(
        const std::string& mediaFolderPath) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".jpg";
        const std::string& path =
            fs::path(mediaFolderPath) / fs::path(fileName);

        ImageIO::Get().SaveImage(path, this->image);

        return path;
    }

    void ImageNotification::Resize(const Size& target) { image.Resize(target); }

    void ImageNotification::Resize(double fx, double fy) {
        Size size = image.GetSize();
        size.width *= fx;
        size.height *= fy;

        this->Resize(size);
    }
}  // namespace Observer
