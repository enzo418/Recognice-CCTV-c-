#include "Rect.hpp"

namespace Observer {
    Rect::Rect(Point tl, Point br)
        : x(tl.x), y(tl.y), width(br.x - tl.x), height(br.y - tl.y) {}

    Rect::Rect(int pX, int pY, int pWidth, int pHeight)
        : x(pX), y(pY), width(pWidth), height(pHeight) {};

    Rect::Rect(int pX, int pY, Size& size)
        : x(pX), y(pY), width(size.width), height(size.height) {};

    Rect::Rect(Point& pPoint, Size& size)
        : x(pPoint.x), y(pPoint.y), width(size.width), height(size.height) {};

    Rect::Rect(Point& pPoint, int pWidth, int pHeight)
        : x(pPoint.x), y(pPoint.y), width(pWidth), height(pHeight) {};

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
}  // namespace Observer