#pragma once

#include <iostream>

#include "IFrame.hpp"

namespace Observer {

    enum ImageWriteFlags {
        // added as needed

        // check this for opencv values
        // https://www.awaresystems.be/imaging/tiff/tifftags/compression.html
        // or https://en.wikipedia.org/wiki/TIFF#TIFF_Compression_Tag
        USE_TIFF_COMPRESSION,

        // image DPI
        USE_TIFF_RESUNIT
    };

    /**
     * @brief Handles Image input/output.
     *
     */
    class IImageIO {
       public:
        /**
         * @brief Writes an image to disk
         *
         * @param path path with the filename + extension
         * @param image
         * @param flags list of flag followed by a value, e.g.
         * {ImageWriteFlags::flag1, value1, ImageWriteFlags::flag2, value2, ...}
         */
        virtual void SaveImage(const std::string& path, Frame& image,
                               const std::vector<int>& flags = {}) = 0;

        /**
         * @brief Saves multiple images into a file
         *
         * @param path path with the filename + extension
         * @param images
         * @param flags same as above
         */
        virtual void SaveImages(const std::string& path,
                                std::vector<Frame>& images,
                                const std::vector<int>& flags = {}) = 0;

        /**
         * @brief Read an image from disk
         *
         * @param path path to read the image from
         * @param imageOut
         */
        virtual void ReadImage(const std::string& path, Frame& imageOut) = 0;

        virtual void ReadImages(const std::string& path,
                                std::vector<Frame>& imagesOut) = 0;
    };
}  // namespace Observer