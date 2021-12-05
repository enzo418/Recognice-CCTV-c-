#pragma once

#include <iostream>

namespace Observer {
    template <typename T>
    struct ImagePersistence;

    template <typename T>
    struct ImagePersistence {
        /**
         * @brief Writes an image to disk
         * 
         * @param path path with the filename
         * @param image 
         */
        static void SaveImage(const std::string& path, T& image);

        /**
         * @brief Read an image from disk
         * 
         * @param path path to read the image from
         * @param imageOut 
         */
        static void ReadImage(const std::string& path, T& imageOut);
    };
}  // namespace Observer