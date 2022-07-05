#pragma once

#include <opencv2/opencv.hpp>

#include "Frame.hpp"
#include "observer/Blob/ImageDrawBlob.hpp"

namespace Observer {
    /**
     * @brief Singleton implementation of image draw blob
     *
     */
    class ImageDrawBlob : public IImageDrawBlob {
       public:
        void DrawBlob(Frame& frame, Blob& blob, int frameNumber,
                      double scaleX = 1, double scaleY = 1) override;

        void DrawBlobs(Frame& frame, std::vector<Blob>& blobs, int frameNumber,
                       double scaleX = 1, double scaleY = 1) override;

        /**
         * @brief Draws all the blobs that appear in each frame.
         *
         * @param frames frames to draw on
         * @param blobs blobs
         * @param scaleX scales x axis
         * @param scaleY scales y axis
         */
        void DrawBlobs(std::vector<Frame>& frames, std::vector<Blob>& blobs,
                       double scaleX = 1, double scaleY = 1) override;

        /**
         * @brief Get the instance
         *
         * @return ImageDrawBlob&
         */
        static ImageDrawBlob& Get();
    };
}  // namespace Observer