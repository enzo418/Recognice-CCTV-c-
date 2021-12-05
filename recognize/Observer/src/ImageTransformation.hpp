#pragma once

#include <opencv2/opencv.hpp>

#include "Rect.hpp"
#include "Size.hpp"

namespace Observer {
    // Forward declaration of the struct
    template <typename T>
    struct ImageTransformation;

    enum ColorSpaceConversion {
        // RGB - Gray
        COLOR_RGB2GRAY = 0,
        COLOR_GRAY2RGB = 1,

        // RGB - HLS
        COLOR_HLS2RGB = 2,
        COLOR_RGB2HLS = 3
    };

    // function for each threhold type:
    // https://docs.opencv.org/4.x/d7/d1b/group__imgproc__misc.html#ggaa9e58d2860d4afa658ef70a9b1115576a147222a96556ebc1d948b372bcd7ac59
    enum ThresholdType {
        THRESHOLD_BINARY = 0,
        THRESHOLD_BINARY_INV = 1,
        THRESHOLD_TRUNC = 2,
        THRESHOLD_TOZERO = 3,
        THRESHOLD_TOZERO_INV = 4
    };

    /**
     * @brief This struct handles all the basics transformations
     * of the selected framework/library
     * @tparam T
     */
    template <typename T>
    struct ImageTransformation {
        /**
         * @brief Stack images Horizontally
         *
         * @param images Array of images to stack
         * @param arraySize Amount of images to concat
         * @param height Height of each image in the array.
         * 0 to automatically calculate it.
         * @return T
         */
        static T HStackPadded(T* images, uint8_t arraySize, uint8_t height);

        /**
         * @brief Stack images Vertically
         *
         * @param images Array of images to stack
         * @param arraySize Amount of images to concat
         * @param width Width of each image in the array. 0 to automatically
         * calculate it.
         * @return T
         */
        static T VStackPadded(T* images, uint8_t arraySize, uint8_t width = 0);

        /**
         * @brief Stack `arraySize/maxHStack` rows vertically.
         * The rows are created from first image to last,
         * where the number of columns is `maxHStack`.
         *
         * @param images Array of images to stack
         * @param arraySize Number of images to stack
         * @param maxHStack Number of images to stack horizontally on each row
         * @return T
         */
        static T StackImages(T* images, uint8_t arraySize,
                             uint8_t maxHStack = 2);

        /**
         * @brief Rotates a image
         *
         * @param image image to rotate
         * @param angle angle, on degrees
         */
        static void RotateImage(T& image, double angle);

        /**
         * @brief Add pad to iamge
         *
         * @param image image to add the pad
         * @param top Pad to add on top of the image
         * @param bottom Pad to add below the image
         * @param left Pad to add in the left side of the image
         * @param right Pad to add in the right side of the image
         */
        static void AddPad(T& image, uint8_t top = 0, uint8_t bottom = 0,
                           uint8_t left = 0, uint8_t right = 0);

        /**
         * @brief Resize a image
         *
         * @param source source image
         * @param dst destionation image
         * @param size size
         */
        static void Resize(T& source, T& dst, const Size& size);

        /**
         * @brief Scales a image
         *
         * @param soruce source image
         * @param dst destionation image
         * @param scaleFactor scale factor
         */
        static void Resize(T& soruce, T& dst, const double scaleFactorX,
                           const double scaleFactorY);

        /**
         * @brief Calculates the absolute difference element by element
         * between two images
         *
         * @param source1
         * @param source2
         * @param dest
         */
        static void AbsoluteDifference(T& source1, T& source2, T& dest);

        /**
         * @brief Blurs an image using a Gaussian filter
         *
         * @param source source image
         * @param dst destination image
         * @param radius
         */
        static void GaussianBlur(T& source, T& dst, int radius);

        /**
         * @brief Converts an image from one color space to another
         *
         * @param source source image
         * @param dst destination image
         * @param conversionType space conversion (ColorSpaceConversion)
         */
        static void ToColorSpace(
            T& source, T& dst,
            int conversionType = ColorSpaceConversion::COLOR_RGB2GRAY);

        /**
         * @brief Copy a image. Deep copy.
         *
         * @param source source to be copied
         * @param dst destination of the copy
         */
        static inline void CopyImage(T& source, T& dst);

        /**
         * @brief Creates a black image.
         *
         * @param reference image to use as a reference while creating the black
         * image (e.g. it will use the same width, height and type)
         */
        static inline cv::Mat BlackImage(T* reference = nullptr);

        /**
         * @brief Crop a image, no data is copied.
         * The destionation image pointer will be pointing to the the sub-array
         * associated with the specified roi.
         *
         * @param source image to be cropped
         * @param dst destionation image
         * @param roi region of interest
         */
        static inline void CropImage(T& source, T& dst, const Rect& roi);

        /**
         * @brief This function applies fixed-level thresholding to a image.
         * The function is typically used to get a binary image out of a
         * grayscale image or for removing a noise, that is, filtering out
         * pixels with too small or too large values. There are several types of
         * thresholding supported by the function. They are determined by
         * type parameter. (description from opencv:threshold)
         *
         * @param source source image
         * @param dst destination image
         * @param threshold
         * @param max value to use when applying THRESHOLD_BINARY
         * or THRESHOLD_BINARY_INV
         * @param type threshold type (ThresholdType)
         */
        static void Threshold(T& source, T& dst, double threshold, double max,
                              int type);

        /**
         * @brief Count the number of non-zero elements in the image
         *
         * @param image Gray image (SINGLE-CHANNEL)
         * @return int count
         */
        static int CountNonZero(T& image);
    };
}  // namespace Observer