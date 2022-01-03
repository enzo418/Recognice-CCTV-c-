#pragma once

#include <cstdint>
#include <tuple>

#include "../vendor/bitmask_operators.hpp"
#include "Point.hpp"
#include "Size.hpp"

namespace Observer {
    struct Rect {
        Rect() = default;

        Rect(Point tl, Point br);

        Rect(int pX, int pY, int pWidth, int pHeight);

        Rect(int pX, int pY, Size& size);

        Rect(Point& pPoint, Size& size);

        Rect(Point& pPoint, int pWidth, int pHeight);

        bool empty();

        int area();

        // x position from the top-left
        int x;

        // y position from the top-left
        int y;

        // width
        int width;

        // height
        int height;

        bool operator==(const Rect& other) const;

        Point tl();

        Point br();

        /**
         * @brief Calculates the intersection with other rect.
         *
         * @param other
         * @return Rect
         */
        Rect Intersection(const Rect& other);

        // We know how to convert from some rect of some library to this one
        template <typename Rect_>
        Rect(const Rect_&);

        // We know how to convert this Rect to some Rect_
        template <typename Rect_>
        operator Rect_();
    };
}  // namespace Observer