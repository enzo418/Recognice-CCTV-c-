#pragma once

#include <iostream>

namespace Observer {
    template <typename T>
    struct ImagePersistence;

    template <typename T>
    struct ImagePersistence {
        void SaveImage(std::string& path, T& image);

        void ReadImage(std::string& path, T& imageOut);
    };
}  // namespace Observer