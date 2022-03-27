#include "ImageIO.hpp"

namespace Observer {
    void ImageIO::SaveImage(const std::string& path, Frame& image) {
        cv::imwrite(path, image.GetInternalFrame());
    }

    void ImageIO::ReadImage(const std::string& path, Frame& imageOut) {
        imageOut.GetInternalFrame() = cv::imread(path);
    }

    ImageIO& ImageIO::Get() {
        static ImageIO io;
        return io;
    }
}  // namespace Observer