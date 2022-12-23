#pragma once

#include <iostream>

#include "IFrame.hpp"

namespace Observer {
    class IImageDisplay {
       public:
        /**
         * @brief Create a new Window
         *
         * @param name name of the new window
         */
        virtual void CreateWindow(const std::string& name) = 0;

        /**
         * @brief Show an image on a named window
         *
         * @param windowName name of the window where to show it
         * @param image
         */
        virtual inline void ShowImage(const std::string& windowName,
                                      Frame& image) = 0;

        /**
         * @brief Destroy a window
         *
         * @param name name of the window
         */
        virtual void DestroyWindow(const std::string& name) = 0;

        // Why stack is here? Since i could not think of another use than
        // displaying the resulting image.

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
        virtual Frame StackImages(Frame* images, int arraySize,
                                  int maxHStack = 2) = 0;
    };
}  // namespace Observer