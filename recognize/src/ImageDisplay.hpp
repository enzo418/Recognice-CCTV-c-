#pragma once

#include <iostream>

namespace Observer {
    template <typename T>
    struct ImageDisplay;

    template <typename T>
    struct ImageDisplay {
        /**
         * @brief Create a new Window
         * 
         * @param name name of the new window
         */
        static void CreateWindow(const std::string& name);

        /**
         * @brief Show an image on a named window
         * 
         * @param windowName name of the window where to show it
         * @param image 
         */
        static inline void ShowImage(const std::string& windowName, T& image);
        
        /**
         * @brief Destroy a window
         * 
         * @param name name of the window
         */
        static void DestroyWindow(const std::string& name);
    };
}  // namespace Observer