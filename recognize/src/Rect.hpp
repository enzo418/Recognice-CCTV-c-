#pragma once

#include "Size.hpp"
#include "Point.hpp"
#include <cstdint>

namespace Observer {
    struct Rect {
        Rect(int pX, int pY, int pWidth, int pHeight)
            : x(pX), y(pY), width(pWidth), height(pHeight) {};

        Rect(int pX, int pY, Size& size)
            : x(pX), y(pY), width(size.width), height(size.height) {};

        Rect(Point& pPoint, Size& size)
            : x(pPoint.x), y(pPoint.y), width(size.width), height(size.height) {};
        
        Rect(Point& pPoint, int pWidth, int pHeight)
            : x(pPoint.x), y(pPoint.y), width(pWidth), height(pHeight) {};

        bool empty() {
            return width == 0 || height == 0;
        }
        
        // x position from the top-left
        int x;
        
        // y position from the top-left
        int y;

        // width
        int width;

        // height
        int height;
    };
}