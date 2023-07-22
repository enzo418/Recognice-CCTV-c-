#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "Rect.hpp"
#include "Size.hpp"

namespace Observer {
    class Frame;

    enum ColorSpaceConversion {
        // RGB - Gray
        COLOR_RGB2GRAY = 0,
        COLOR_GRAY2RGB = 1,

        // RGB - HLS
        COLOR_HLS2RGB = 2,
        COLOR_RGB2HLS = 3,

        // RGB - BGR
        COLOR_RGB2BGR = 4,
        COLOR_BGR2RGB = 5,
    };

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

    /**
     * @brief This interface defines basics operations that a frame can handle.
     * The constructor of a frame would need at lest 3 different ones:
     *      1. Frame(): Creates an empty frame
     *      2. Frame(size, channels): Creates a frame with a size and number of
     *         channels, filled with zeros.
     */
    class IFrame {
       public:
        /**
         * @brief Clone this frame
         *
         * @return Frame
         */
        virtual Frame Clone() = 0;

        /**
         * @brief Copy this frame into another one
         *
         * @param dst
         */
        virtual void CopyTo(Frame& dst) = 0;

        /**
         * @brief Creates a black image with the same size and type of
         * this one.
         *
         * @return Frame
         */
        virtual Frame GetBlackImage() = 0;

        /**
         * @brief Rotates a image
         *
         * @param angle angle, on degrees
         */
        virtual void RotateImage(double angle) = 0;

        /**
         * @brief Resize a image
         *
         * @param size size
         */
        virtual void Resize(const Size& size) = 0;

        /**
         * @brief Scales a image.
         *
         * @param scaleFactor scale factor
         */
        virtual void Resize(const double scaleFactorX,
                            const double scaleFactorY) = 0;

        virtual Frame AbsoluteDifference(Frame& source2) = 0;

        /**
         * @brief Crop a image, no data is copied.
         * The destionation image pointer will be pointing to the the sub-array
         * associated with the specified roi.
         *
         * @param roi region of interest
         */
        virtual void CropImage(const Rect& roi) = 0;

        /**
         * @brief Blurs an image using a Gaussian filter
         *
         * @param radius
         */
        virtual void GaussianBlur(int radius) = 0;

        /**
         * @brief Converts an image from one color space to another
         *
         * @param source source image
         * @param dst destination image
         * @param conversionType space conversion (ColorSpaceConversion)
         */
        virtual void ToColorSpace(
            int conversionType = ColorSpaceConversion::COLOR_RGB2GRAY) = 0;

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
         * @param type threshold type (ThresholdType)
         */
        virtual void Threshold(double threshold, double max, int type) = 0;

        /**
         * @brief Count the number of non-zero elements in the image
         * (SINGLE-CHANNEL).
         *
         * @return int count
         */
        virtual int CountNonZero() = 0;

        /**
         * @brief Get the Size object
         *
         * @param image
         * @return Size
         */
        virtual Size GetSize() = 0;

        /**
         * @brief Adds one image to this one. They need to be the same number
         * of channels.
         *
         * @param dst
         */
        virtual void Add(Frame& imageToAdd) = 0;

        /**
         * @brief Encodes the image.
         *
         * @param ext File extension that defines the output format
         * @param quality Quality of the resulting image, 0-100, 100 is best
         * quality.
         * @param buffer Output buffer.
         * @return true if the image was encoded successfully
         */
        virtual bool EncodeImage(const std::string& ext, int quality,
                                 std::vector<unsigned char>& buffer) = 0;

        virtual bool IsEmpty() = 0;

        virtual void BitwiseNot() = 0;

        virtual void Mask(Frame& mask) = 0;

        virtual uint8_t* GetData() = 0;
    };
}  // namespace Observer