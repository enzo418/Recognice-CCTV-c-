#include "Point.hpp"

namespace Observer {
    Point::Point() : x(0), y(0) {}
    Point::Point(int pX, int pY) : x(pX), y(pY) {};

    bool Point::operator==(const Point& other) const {
        return std::tie(x, y) == std::tie(other.x, other.y);
    }

    void Point::operator+=(const Point& b) {
        this->x += b.x;
        this->y += b.y;
    }

    void Point::operator/=(const int& b) {
        this->x /= b;
        this->y /= b;
    }

    void Point::operator*=(const int& b) {
        this->x *= b;
        this->y *= b;
    }

    double Point::DistanceTo(const Point& other) {
        const int x = other.x - this->x;
        const int y = other.y - this->y;

        return sqrt(x * x + y * y);
    }

    Point operator+(const Point& a, const Point& b) {
        return Point(a.x + b.x, a.y + b.y);
    }

    Point operator/(const Point& a, int b) { return Point(a.x / b, a.y / b); }

    Point operator*(const Point& a, int b) { return Point(a.x * b, a.y * b); }

    double DistanceTo(const Point& a, const Point& b) {
        const int x = b.x - a.x;
        const int y = b.y - a.y;

        return sqrt(x * x + y * y);
    }

    std::ostream& operator<<(std::ostream& os, const Point& pt) {
        os << "[" << pt.x << ", " << pt.y << "]";
        return os;
    }
}  // namespace Observer