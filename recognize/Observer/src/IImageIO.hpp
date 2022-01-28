#pragma once

#include <iostream>

#include "IFrame.hpp"

namespace Observer {

    /**
     * @brief Handles Image input/output.
     *
     */
    class IImageIO {
       public:
        /**
         * @brief Writes an image to disk
         *
         * @param path path with the filename
         * @param image
         */
        virtual void SaveImage(const std::string& path, Frame& image) = 0;

        /**
         * @brief Read an image from disk
         *
         * @param path path to read the image from
         * @param imageOut
         */
        virtual void ReadImage(const std::string& path, Frame& imageOut) = 0;
    };
}  // namespace Observer