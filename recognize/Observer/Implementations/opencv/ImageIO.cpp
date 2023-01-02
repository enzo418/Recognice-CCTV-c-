#include "ImageIO.hpp"

#include <algorithm>
#include <opencv2/imgcodecs.hpp>
#include <stdexcept>

#include "Frame.hpp"

namespace Observer {
    std::vector<int> inline parseFlags(const std::vector<int>& flags) {
        std::vector<int> parsed(flags);

        for (size_t i = 0; i < parsed.size(); i += 2) {
            switch (parsed[i]) {
                case ImageWriteFlags::USE_TIFF_COMPRESSION:
                    parsed[i] = cv::IMWRITE_TIFF_COMPRESSION;
                    break;
                case ImageWriteFlags::USE_TIFF_RESUNIT:
                    parsed[i] = cv::IMWRITE_TIFF_RESUNIT;
                    break;
                default:
                    throw std::runtime_error("Uknown IMAGEIO flag");
            }
        }

        return parsed;
    }

    void ImageIO::SaveImage(const std::string& path, Frame& image,
                            const std::vector<int>& flags) {
        std::vector<int> parsed = parseFlags(flags);

        cv::imwrite(path, image.GetInternalFrame(), parsed);
    }

    void ImageIO::SaveImages(const std::string& path,
                             std::vector<Frame>& images,
                             const std::vector<int>& flags) {
        std::vector<int> parsedFlags = parseFlags(flags);

        // opencv imwrite takes an InputArray which, as its name implies,
        // doesn't allow to be constructed by non contiguous cv::Mat list
        std::vector<cv::Mat> contiguousMat(images.size());
        for (size_t i = 0; i < images.size(); i++) {
            // supposedly by opencv = operator it's O(1) so no copy is made
            contiguousMat[i] = images[i].GetInternalFrame();
        }

        cv::imwrite(path, contiguousMat, parsedFlags);
    }

    void ImageIO::ReadImage(const std::string& path, Frame& imageOut) {
        imageOut.GetInternalFrame() = cv::imread(path);
    }

    void ImageIO::ReadImages(const std::string& path,
                             std::vector<Frame>& imagesOut) {
        std::vector<cv::Mat> frames;

        cv::imreadmulti(path, frames);

        imagesOut.resize(frames.size());

        std::move(frames.begin(), frames.end(), imagesOut.begin());
    }

    ImageIO& ImageIO::Get() {
        static ImageIO io;
        return io;
    }
}  // namespace Observer