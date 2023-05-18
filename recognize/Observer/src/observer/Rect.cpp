#include "Rect.hpp"

namespace Observer {
    Rect::Rect() : x(0), y(0), width(0), height(0) {}

    Rect::Rect(Point tl, Point br)
        : x(tl.x), y(tl.y), width(br.x - tl.x), height(br.y - tl.y) {}

    Rect::Rect(int pX, int pY, int pWidth, int pHeight)
        : x(pX), y(pY), width(pWidth), height(pHeight) {}

    Rect::Rect(int pX, int pY, const Size& size)
        : x(pX), y(pY), width(size.width), height(size.height) {}

    Rect::Rect(const Point& pPoint, const Size& size)
        : x(pPoint.x), y(pPoint.y), width(size.width), height(size.height) {}

    Rect::Rect(const Point& pPoint, int pWidth, int pHeight)
        : x(pPoint.x), y(pPoint.y), width(pWidth), height(pHeight) {}

    Rect Rect::Intersection(const Rect& other) {
        Rect inter;

        int x1 = std::max(this->x, other.x);
        int y1 = std::max(this->y, other.y);

        inter.width =
            std::min(this->x + this->width, other.x + other.width) - x1;
        inter.height =
            std::min(this->y + this->height, other.y + other.height) - y1;

        inter.x = x1;
        inter.y = y1;

        if (inter.width <= 0 || inter.height <= 0) inter = Rect();

        return inter;
    }

    bool Rect::operator==(const Rect& other) const {
        return std::tie(x, y, width, height) ==
               std::tie(other.x, other.y, other.width, other.height);
    }

    bool Rect::empty() { return width == 0 || height == 0; }

    int Rect::area() { return width * height; }

    Point Rect::tl() { return Point(x, y); }

    Point Rect::br() { return Point(x + width, y + height); }

    std::ostream& operator<<(std::ostream& os, const Rect& rt) {
        os << "[P=(" << rt.x << ", " << rt.y << ") Size=(" << rt.width << " x "
           << rt.height << ")]";

        return os;
    }

    Rect Rect::Union(const Rect& other) {
        Rect union_;

        int x1 = std::min(this->x, other.x);
        int y1 = std::min(this->y, other.y);

        union_.width =
            std::max(this->x + this->width, other.x + other.width) - x1;
        union_.height =
            std::max(this->y + this->height, other.y + other.height) - y1;

        union_.x = x1;
        union_.y = y1;

        return union_;
    }

    float Rect::IntersectionOverUnion(const Rect& other) {
        auto inter = this->Intersection(other);
        auto union_ = this->Union(other);

        return (float)inter.area() / union_.area();
    }
}  // namespace Observer