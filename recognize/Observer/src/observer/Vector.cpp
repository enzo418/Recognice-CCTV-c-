#include "Vector.hpp"

namespace Observer {

    double Vec::magnitude() { return this->p1.DistanceTo(this->p2); }

    bool Vec::isNull() {
        return this->p1 == Point(0, 0) && this->p2 == Point(0, 0);
    }

    static Vec& operator+=(Vec& a, const Vec& b) {
        a.p1 += b.p1;
        a.p2 += b.p2;
        return a;
    }

    static Vec operator+(const Vec& a, const Vec& b) {
        return Vec(a.p1 + b.p1, a.p2 + b.p2);
    }

    static Vec operator/(const Vec& a, const int& b) {
        return Vec(a.p1 / b, a.p2 / b);
    }

    static Vec operator/(const Vec& a, const double& b) {
        return Vec(a.p1 / b, a.p2 / b);
    }

    static Vec& operator/=(Vec& a, const int& b) {
        a.p1 /= b;
        a.p2 /= b;
        return a;
    }

    static Vec& operator/=(Vec& a, const double& b) {
        a.p1 /= b;
        a.p2 /= b;
        return a;
    }

    static Vec& operator*=(Vec& a, const int& b) {
        a.p1 *= b;
        a.p2 *= b;
        return a;
    }

    static Vec& operator*=(Vec& a, const double& b) {
        a.p1 *= b;
        a.p2 *= b;
        return a;
    }
}  // namespace Observer