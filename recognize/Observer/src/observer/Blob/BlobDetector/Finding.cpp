#include "Finding.hpp"

namespace Observer {

    Finding::Finding(Point p) {
        /**
         * Since it's the first point, we need to set the tl and br points to
         * this point. Else, (0,0) will be used on AddPoint and min will always
         * be (0,0) for all findings.
         */
        this->tl = p;
        this->br = p;
        this->AddPoint(p);
    };

    void Finding::AddPoint(Point& p) {
        this->tl = Point(std::min(this->tl.x, p.x), std::min(this->tl.y, p.y));
        this->br = Point(std::max(this->br.x, p.x), std::max(this->br.y, p.y));
        this->points.push_back(p);
    }

    double Finding::GetShortestDistance(Point& p) {
        int vx = std::max(std::min(p.x, br.x), tl.x);
        int vy = std::max(std::min(p.y, br.y), tl.y);

        return p.DistanceTo(Point(vx, vy));
    }

    std::vector<Point> Finding::TakePoints() { return std::move(this->points); }

    std::vector<Point>& Finding::GetPoints() { return this->points; }

    Rect Finding::GetBoundingRect() { return Rect(this->tl, this->br); }

}  // namespace Observer