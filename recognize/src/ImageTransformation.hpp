#pragma once

#include <opencv2/opencv.hpp>
namespace Observer {
    // Forward declaration of the struct
    template <typename T>
    struct ImageTransformation;

    /**
     * @brief This struct handles all the basics transformations
     * of the selected framework/library
     * @tparam T 
     */
    template<typename T>
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
        * @param width Width of each image in the array. 0 to automatically calculate it.
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
        static void AddPad(T& image, uint8_t top = 0, uint8_t bottom = 0, uint8_t left = 0, uint8_t right = 0);
    };
}