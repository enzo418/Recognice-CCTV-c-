#pragma once

#include <cmath>
#include <cstdint>
#include <tuple>

namespace Observer {
    struct Point {
        Point() = default;
        Point(int pX, int pY) : x(pX), y(pY) {};

        int x;
        int y;

        bool operator==(const Point& other) const {
            return std::tie(x, y) == std::tie(other.x, other.y);
        }

        void operator+=(const Point& b) {
            this->x += b.x;
            this->y += b.y;
        }

        void operator/=(const int& b) {
            this->x /= b;
            this->y /= b;
        }

        void operator*=(const int& b) {
            this->x *= b;
            this->y *= b;
        }

        double DistanceTo(const Point& other) {
            const int x = other.x - this->x;
            const int y = other.y - this->y;

            return sqrt(x * x + y * y);
        }

        // We know how to convert from some point of some library to this one
        template <typename Point_>
        Point(const Point_&);

        // We know how to convert this Point to some Point_
        template <typename Point_>
        operator Point_();
    };

    static Point operator+(const Point& a, const Point& b) {
        return Point(a.x + b.x, a.y + b.y);
    }

    static Point operator/(const Point& a, int b) {
        return Point(a.x / b, a.y / b);
    }

    static Point operator*(const Point& a, int b) {
        return Point(a.x * b, a.y * b);
    }

    static double DistanceTo(const Point& a, const Point& b) {
        const int x = b.x - a.x;
        const int y = b.y - a.y;

        return sqrt(x * x + y * y);
    }
}  // namespace Observer