#pragma once

#include <cuchar>
#include <memory>
#include <vector>

#include "Size.hpp"

namespace Observer {

    // function for each threhold type:
    // https://docs.opencv.org/4.x/d7/d1b/group__imgproc__misc.html#ggaa9e58d2860d4afa658ef70a9b1115576a147222a96556ebc1d948b372bcd7ac59
    enum ThresholdType {
        THRESHOLD_BINARY = 1,
        THRESHOLD_BINARY_INV = 2,
        THRESHOLD_TRUNC = 4,
        THRESHOLD_TOZERO = 6,
        THRESHOLD_TOZERO_INV = 8,
        THRESHOLD_TRIANGLE = 16
    };

    class IFrame {
       public:
        /**
         * @brief Clone an image
         *
         * @return IFrame
         */
        virtual std::unique_ptr<IFrame> Clone() = 0;

        /**
         * @brief Copy this image.
         *
         * @param dst destination of the copy
         */
        virtual void CopyTop(IFrame& dst) = 0;

        /**
         * @brief Rotate this image.
         *
         * @param degrees
         */
        virtual void Rotate(double degrees) = 0;

        /**
         * @brief Calculates the absolute differente with frame2.
         *
         * @param frame2 frame to calculate the difference with frame.
         * @param dst frame where the difference is stored
         */
        virtual void AbsoluteDifference(IFrame& frame2, IFrame& dst) = 0;

        // virtual static void BlackImage();

        /**
         * @brief Resize the frame
         *
         * @param size target size
         */
        virtual void Resize(Size& size);

        /**
         * @brief Resize the frame
         * If factorx = 2, makes the image twice the witdh.
         *
         * @param factorX target factor x
         * @param factorY target factor y
         */
        virtual void Resize(double factorX, double factorY);

        /**
         * @brief This function applies fixed-level thresholding to a image.
         * The function is typically used to get a binary image out of a
         * grayscale image or for removing a noise, that is, filtering out
         * pixels with too small or too large values. There are several types of
         * thresholding supported by the function. They are determined by
         * type parameter. (description from opencv:threshold)
         *
         * @param threshold
         * @param max value to use when applying THRESHOLD_BINARY
         * or THRESHOLD_BINARY_INV
         * @param type See ThresholdType
         */
        virtual void Threshold(double threshold, double max, int type);

        /**
         * @brief Count the number of non-zero elements in the image.
         * This frame need to be SINGLE-CHANNEL.
         *
         * @return int count
         */
        virtual int CountNonZero();

        /**
         * @brief Get the Size object
         *
         * @param image
         * @return Size
         */
        virtual Size GetSize();

        /**
         * @brief Adds a image and this one into this one.
         *
         * @param image2
         */
        virtual void AddImages(IFrame& image2);

        /**
         * @brief Adds a image to this one into another image.
         * This method is an overload of the one above.
         *
         * @param image2
         * @param dst destination.
         */
        virtual void AddImages(IFrame& image2, IFrame& dst);

        /**
         * @brief Encode this image.
         *
         * @param ext File extension that defines the output format
         * @param quality Quality of the resulting image, 0-100, 100 is best
         * quality.
         * @param buffer Output buffer.
         */
        static inline void EncodeImage(const std::string& ext, int quality,
                                       std::vector<unsigned char>& buffer);
    };
}  // namespace Observer