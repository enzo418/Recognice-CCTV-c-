#pragma once

#include <cstdint>
#include <tuple>

namespace Observer {
    struct Point {
        Point(int pX, int pY) : x(pX), y(pY) {};

        int x;
        int y;

        bool operator==(const Point& other) const {
            return std::tie(x, y) == std::tie(other.x, other.y);
        }
    };
}  // namespace Observer