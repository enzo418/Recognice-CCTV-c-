#include "Math.hpp"

#include <limits>

namespace Observer {
    double map(double x, double in_min, double in_max, double out_min,
               double out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    double lerp(double start, double end, double t) {
        return start * (1 - t) + end * t;
    }

    std::vector<Point> Interpolate(Point start, Point end, int max_points) {
        std::vector<Point> points;
        for (int step = 1; step < max_points; step++) {
            double t = map(step, 0, max_points, 0, 1);
            int x = floor(lerp(start.x, end.x, t));
            int y = floor(lerp(start.y, end.y, t));
            points.push_back(Point(x, y));
        }

        return points;
    }

    Rect BoundingRect(const std::vector<Point>& points) {
        Point tl(std::numeric_limits<int>::max(),
                 std::numeric_limits<int>::max());

        Point br(std::numeric_limits<int>::min(),
                 std::numeric_limits<int>::min());

        for (auto& point : points) {
            if (point.x < tl.x) tl.x = point.x;
            if (point.x > br.x) br.x = point.x;

            if (point.y < tl.y) tl.y = point.y;
            if (point.y > br.y) br.y = point.y;
        }

        return Rect(tl, br);
    }
}  // namespace Observer