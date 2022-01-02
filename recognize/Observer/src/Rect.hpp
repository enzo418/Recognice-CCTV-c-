#pragma once

#include <cstdint>
#include <tuple>

#include "Point.hpp"
#include "Size.hpp"

namespace Observer {
    struct Rect {
        Rect() = default;

        Rect(Point tl, Point br)
            : x(tl.x), y(tl.y), width(br.x - tl.x), height(br.y - tl.y) {}

        Rect(int pX, int pY, int pWidth, int pHeight)
            : x(pX), y(pY), width(pWidth), height(pHeight) {};

        Rect(int pX, int pY, Size& size)
            : x(pX), y(pY), width(size.width), height(size.height) {};

        Rect(Point& pPoint, Size& size)
            : x(pPoint.x),
              y(pPoint.y),
              width(size.width),
              height(size.height) {};

        Rect(Point& pPoint, int pWidth, int pHeight)
            : x(pPoint.x), y(pPoint.y), width(pWidth), height(pHeight) {};

        bool empty() { return width == 0 || height == 0; }

        int area() { return width * height; }

        // x position from the top-left
        int x;

        // y position from the top-left
        int y;

        // width
        int width;

        // height
        int height;

        bool operator==(const Rect& other) const {
            return std::tie(x, y, width, height) ==
                   std::tie(other.x, other.y, other.width, other.height);
        }

        Point tl() { return Point(x, y); }

        Point br() { return Point(x + width, y + height); }

        // We know how to convert from some rect of some library to this one
        template <typename Rect_>
        Rect(const Rect_&);

        // We know how to convert this Rect to some Rect_
        template <typename Rect_>
        operator Rect_();
    };

    /** From
     * https://github.com/opencv/opencv/blob/d24befa0bc7ef5e73bf8b1402fa1facbdbf9febb/modules/core/include/opencv2/core/types.hpp#L1896
     */
    static Rect& operator&=(Rect& a, const Rect& b) {
        int x1 = std::max(a.x, b.x);
        int y1 = std::max(a.y, b.y);
        a.width = std::min(a.x + a.width, b.x + b.width) - x1;
        a.height = std::min(a.y + a.height, b.y + b.height) - y1;
        a.x = x1;
        a.y = y1;
        if (a.width <= 0 || a.height <= 0) a = Rect();
        return a;
    }

    static Rect operator&(const Rect& a, const Rect& b) {
        Rect c = a;
        return c &= b;
    }
}  // namespace Observer