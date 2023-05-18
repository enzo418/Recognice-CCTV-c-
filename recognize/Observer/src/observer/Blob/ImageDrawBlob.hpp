#pragma once

#include "BlobDetector/Blob.hpp"
#include "observer/Domain/Classification/BlobClassification.hpp"
#include "observer/IFrame.hpp"

namespace Observer {

    class IImageDrawBlob {
       public:
        /**
         * @brief Draws a blob on a frame.
         *
         * @param frame
         * @param blob
         * @param classification blob classification. Pass null if not
         * available.
         * @param frameNumber
         * @param scaleX
         * @param scaleY
         */
        virtual void DrawBlob(Frame& frame, Blob& blob,
                              const BlobClassification* classification,
                              int frameNumber, double scaleX = 1,
                              double scaleY = 1) = 0;

        virtual void DrawBlobs(Frame& frame, std::vector<Blob>& blobs,
                               const BlobClassifications& classifications,
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
                               std::vector<Blob>& blobs,
                               const BlobClassifications& classifications,
                               double scaleX = 1, double scaleY = 1) = 0;
    };
}  // namespace Observer