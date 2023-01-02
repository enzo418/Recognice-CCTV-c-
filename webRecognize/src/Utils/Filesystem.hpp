#pragma once

#include <filesystem>

namespace Web::Filesystem {

    /**
     * @brief Get the directory size in bytes, if it doesn't exist returns 0.
     *
     * @param path
     * @return uintmax_t
     */
    inline uintmax_t getDirectorySize(const std::filesystem::path& path) {
        uintmax_t totalSize = 0;
        if (!std::filesystem::exists(path)) {
            // directory does not exist
            return 0;
        }

        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }

        return totalSize;
    }
}  // namespace Web::Filesystem