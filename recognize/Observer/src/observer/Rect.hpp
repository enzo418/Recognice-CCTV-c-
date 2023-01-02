#pragma once

#include <cstdint>
#include <tuple>

#include "../../vendor/bitmask_operators.hpp"
#include "Point.hpp"
#include "Size.hpp"

template <typename T>
concept RectType = requires(T a) {
                       a.x;
                       a.y;
                       a.width;
                       a.height;
                   };

namespace Observer {
    struct Rect {
        Rect();

        Rect(Point tl, Point br);

        Rect(int pX, int pY, int pWidth, int pHeight);

        Rect(int pX, int pY, const Size& size);

        Rect(const Point& pPoint, const Size& size);

        Rect(const Point& pPoint, int pWidth, int pHeight);

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
        template <RectType Rect_>
        Rect(const Rect_&);

        // We know how to convert this Rect to some Rect_
        template <typename Rect_>
            requires RectType<Rect_>
        operator Rect_();

        // same for const
        template <typename Rect_>
            requires RectType<Rect_>
        operator Rect_() const;
    };

    std::ostream& operator<<(std::ostream& os, const Rect& pt);
}  // namespace Observer