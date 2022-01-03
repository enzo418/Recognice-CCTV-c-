#pragma once

#include <cmath>
#include <cstdint>
#include <tuple>

namespace Observer {
    struct Point {
        Point();
        Point(int pX, int pY);

        int x;
        int y;

        bool operator==(const Point& other) const;

        void operator+=(const Point& b);

        void operator/=(const int& b);

        void operator*=(const int& b);

        double DistanceTo(const Point& other);

        // We know how to convert from some point of some library to this one
        template <typename Point_>
        Point(const Point_&);

        // We know how to convert this Point to some Point_
        template <typename Point_>
        operator Point_();
    };

    static Point operator+(const Point& a, const Point& b);

    static Point operator/(const Point& a, int b);

    static Point operator*(const Point& a, int b);

    static double DistanceTo(const Point& a, const Point& b);
}  // namespace Observer