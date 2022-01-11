#pragma once

#include <vector>

#include "BlobDetector/Blob.hpp"

namespace Observer {
    template <typename T>
    struct BlobGraphics;

    template <typename T>
    struct BlobGraphics {
        static void DrawBlob(T& frame, Blob& blob, int frameNumber,
                             double scaleX = 1, double scaleY = 1);

        static void DrawBlobs(T& frame, std::vector<Blob>& blobs,
                              int frameNumber, double scaleX = 1,
                              double scaleY = 1);

        /**
         * @brief Draws all the blobs that appear in each frame.
         *
         * @param frames frames to draw on
         * @param blobs blobs
         * @param scaleX scales x axis
         * @param scaleY scales y axis
         */
        static void DrawBlobs(std::vector<T>& frames, std::vector<Blob>& blobs,
                              double scaleX = 1, double scaleY = 1);
    };
}  // namespace Observer