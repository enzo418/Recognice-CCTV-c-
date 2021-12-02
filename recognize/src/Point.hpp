#pragma once

#include <cstdint>

namespace Observer {
    struct Point {
        Point(int16_t pX, int16_t pY)
            : x(pX), y(pY) {};

        int16_t x;
        int16_t y;
    };
}