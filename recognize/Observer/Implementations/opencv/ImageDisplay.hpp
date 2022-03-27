#pragma once

#include <opencv2/highgui.hpp>

#include "../../src/IImageDisplay.hpp"
#include "Frame.hpp"

namespace Observer {
    class ImageDisplay : public IImageDisplay {
       public:
        /**
         * @brief Create a new Window
         *
         * @param name name of the new window
         */
        void CreateWindow(const std::string& name) override;

        /**
         * @brief Show an image on a named window
         *
         * @param windowName name of the window where to show it
         * @param image
         */
        void ShowImage(const std::string& windowName, Frame& image) override;

        /**
         * @brief Destroy a window
         *
         * @param name name of the window
         */
        void DestroyWindow(const std::string& name) override;

        /**
         * @brief Stack `arraySize/maxHStack` rows vertically.
         * The rows are created from first image to last,
         * where the number of columns is `maxHStack`.
         *
         * @param images Array of images to stack
         * @param arraySize Number of images to stack
         * @param maxHStack Number of images to stack horizontally on each row
         * @return Frame
         */
        Frame StackImages(Frame* images, uint8_t arraySize,
                          uint8_t maxHStack = 2) override;

        /**
         * @brief Get the instance
         *
         * @return ImageDisplay&
         */
        static ImageDisplay& Get();

       private:
        cv::Mat InternalStackImages(cv::Mat* images, uint8_t arraySize,
                                    uint8_t maxHStack = 2);

        /**
         * @brief Stack images Horizontally
         *
         * @param images Array of images to stack
         * @param arraySize Amount of images to concat
         * @param height Height of each image in the array.
         * 0 to automatically calculate it.
         * @return Frame
         */
        cv::Mat HStackPadded(cv::Mat* images, uint8_t arraySize,
                             uint8_t height);

        /**
         * @brief Stack images Vertically
         *
         * @param images Array of images to stack
         * @param arraySize Amount of images to concat
         * @param width Width of each image in the array. 0 to automatically
         * calculate it.
         * @return Frame
         */
        cv::Mat VStackPadded(cv::Mat* images, uint8_t arraySize,
                             uint8_t width = 0);

        /**
         * @brief Add pad to iamge
         *
         * @param image image to add the pad
         * @param top Pad to add on top of the image
         * @param bottom Pad to add below the image
         * @param left Pad to add in the left side of the image
         * @param right Pad to add in the right side of the image
         */
        void AddPad(cv::Mat& image, uint8_t top = 0, uint8_t bottom = 0,
                    uint8_t left = 0, uint8_t right = 0);
    };
}  // namespace Observer