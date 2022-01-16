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

    double PointPolygonTest(std::vector<Point>& points, Point& ip) {
        double result = 0;
        int total = points.size();
        int counter = 0;

        if (total == 0) return -1;

        Point v0, v = points[total - 1];

        for (int i = 0; i < total; i++) {
            v0 = v;
            v = points[i];

            if ((v0.y <= ip.y && v.y <= ip.y) || (v0.y > ip.y && v.y > ip.y) ||
                (v0.x < ip.x && v.x < ip.x)) {
                if (ip.y == v.y &&
                    (ip.x == v.x ||
                     (ip.y == v0.y && ((v0.x <= ip.x && ip.x <= v.x) ||
                                       (v.x <= ip.x && ip.x <= v0.x)))))
                    return 0;
                continue;
            }

            double dist = round((ip.y - v0.y) * (v.x - v0.x)) -
                          round((ip.x - v0.x) * (v.y - v0.y));

            if (dist == 0) return 0;
            if (v.y < v0.y) dist = -dist;

            counter += dist > 0;
        }

        result = counter % 2 == 0 ? -1 : 1;
        return result;
    }
}  // namespace Observer