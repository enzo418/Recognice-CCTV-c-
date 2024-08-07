#pragma once

#include <opencv2/opencv.hpp>

#include "Frame.hpp"
#include "observer/IImageIO.hpp"

namespace Observer {

    /**
     * @brief Singleton implementation of IImageIO.
     *
     */
    class ImageIO final : public IImageIO {
       public:
        /**
         * @brief Writes an image to disk
         *
         * @param path path with the filename
         * @param image
         */
        void SaveImage(const std::string& path, Frame& image,
                       const std::vector<int>& flags = {}) override;

        void SaveImages(const std::string& path, std::vector<Frame>& images,
                        const std::vector<int>& flags = {}) override;

        /**
         * @brief Read an image from disk
         *
         * @param path path to read the image from
         * @param imageOut
         */
        void ReadImage(const std::string& path, Frame& imageOut) override;

        void ReadImages(const std::string& path,
                        std::vector<Frame>& imagesOut) override;

        void ReadDecode(const std::vector<uint8_t>& data,
                        Frame& imageOut) override;

        /**
         * @brief Get the instance
         *
         * @return ImageIO
         */
        static ImageIO& Get();
    };
}  // namespace Observer