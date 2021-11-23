#pragma once

#include <opencv2/opencv.hpp>

template<typename T>
class IImageTransformation {
   public:
    /**
    * @brief Stack images Horizontally
    *
    * @param images Array of images to stack
    * @param arraySize Amount of images to concat
    * @param height Height of each image in the array.
    * 0 to automatically calculate it.
    * @return T 
    */
    virtual T HStackPadded(T* images, uint8_t arraySize, uint8_t height) = 0;

    /**
     * @brief Stack images Vertically
     * 
     * @param images Array of images to stack
     * @param arraySize Amount of images to concat
     * @param width Width of each image in the array. 0 to automatically calculate it.
     * @return T 
     */
    virtual T VStackPadded(T* images, uint8_t arraySize, uint8_t width = 0) = 0;

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
    virtual T StackImages(T* images, uint8_t arraySize,
                                uint8_t maxHStack = 2) = 0;

    /**
     * @brief Rotates a image
     * 
     * @param image image to rotate
     * @param angle angle, on degrees
     */
    virtual void RotateImage(T& image, double angle) = 0;
};