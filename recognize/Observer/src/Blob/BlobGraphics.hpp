#pragma once

#include <vector>

#include "BlobDetector/Blob.hpp"

namespace Observer {
    template <typename T>
    struct BlobGraphics;

    template <typename T>
    struct BlobGraphics {
        static void DrawBlob(T& frame, Blob& blob, int frameNumber);

        static void DrawBlobs(T& frame, std::vector<Blob>& blobs,
                              int frameNumber);

        static void DrawBlobs(std::vector<T>& frame, std::vector<Blob>& blobs);
    };
}  // namespace Observer