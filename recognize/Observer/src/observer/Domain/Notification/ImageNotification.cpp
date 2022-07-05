#include "ImageNotification.hpp"

namespace Observer {

    namespace fs = std::filesystem;

    ImageNotification::ImageNotification(int pGroupID, Event pEvent,
                                         Frame& frame,
                                         std::string pOutputFolder)
        : Notification(
              pGroupID, std::move(pEvent),
              std::move(ImageNotification::CreatePath(pOutputFolder))) {
        frame.CopyTo(image);
    }

    Frame& ImageNotification::GetImage() { return this->image; }

    void ImageNotification::BuildNotification() {
        // GetContent is the path to the image
        ImageIO::Get().SaveImage(this->GetContent(), this->image);
    }

    void ImageNotification::Resize(const Size& target) { image.Resize(target); }

    void ImageNotification::Resize(double fx, double fy) {
        Size size = image.GetSize();
        size.width *= fx;
        size.height *= fy;

        this->Resize(size);
    }

    std::string ImageNotification::CreatePath(
        const std::string& pOutputFolder) {
        const std::string time = Observer::SpecialFunctions::GetCurrentTime();
        const std::string fileName = time + ".jpg";
        const std::string& path = fs::path(pOutputFolder) / fs::path(fileName);
        return path;
    }
}  // namespace Observer
