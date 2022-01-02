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

    static Vec& operator+=(Vec& a, const Vec& b);

    static Vec operator+(const Vec& a, const Vec& b);
    static Vec operator/(const Vec& a, const int& b);
    static Vec operator/(const Vec& a, const double& b);

    static Vec& operator/=(Vec& a, const int& b);
    static Vec& operator/=(Vec& a, const double& b);

    static Vec& operator*=(Vec& a, const int& b);
    static Vec& operator*=(Vec& a, const double& b);

}  // namespace Observer