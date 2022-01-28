#pragma once

#include "../IFrame.hpp"
#include "BlobDetector/Blob.hpp"

namespace Observer {

    class IImageDrawBlob {
       public:
        virtual void DrawBlob(Frame& frame, Blob& blob, int frameNumber,
                              double scaleX = 1, double scaleY = 1) = 0;

        virtual void DrawBlobs(Frame& frame, std::vector<Blob>& blobs,
                               int frameNumber, double scaleX = 1,
                               double scaleY = 1) = 0;

        /**
         * @brief Draws all the blobs that appear in each frame.
         *
         * @param frames frames to draw on
         * @param blobs blobs
         * @param scaleX scales x axis
         * @param scaleY scales y axis
         */
        virtual void DrawBlobs(std::vector<Frame>& frames,
                               std::vector<Blob>& blobs, double scaleX = 1,
                               double scaleY = 1) = 0;
    };
}  // namespace Observer