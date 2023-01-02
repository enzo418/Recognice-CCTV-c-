#pragma once

#include "Point.hpp"

namespace Observer {

    struct Vec {
        Vec() = default;
        Vec(Point p1, Point p2) : p1(p1), p2(p2) {}
        Point p1;
        Point p2;

        double magnitude();

        bool isNull();
    };

}  // namespace Observer