#pragma once

#include <opencv2/opencv.hpp>

#include "../../src/IImageIO.hpp"
#include "Frame.hpp"

namespace Observer {

    /**
     * @brief Singleton implementation of IImageIO.
     *
     */
    class ImageIO : public IImageIO {
       public:
        /**
         * @brief Writes an image to disk
         *
         * @param path path with the filename
         * @param image
         */
        void SaveImage(const std::string& path, Frame& image) override;

        /**
         * @brief Read an image from disk
         *
         * @param path path to read the image from
         * @param imageOut
         */
        void ReadImage(const std::string& path, Frame& imageOut) override;

        /**
         * @brief Get the instance
         *
         * @return ImageIO
         */
        static ImageIO& Get();
    };
}  // namespace Observer