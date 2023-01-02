#pragma once

#include <vector>

#include "observer/Point.hpp"
#include "observer/Rect.hpp"

namespace Observer {
    /**
     * @brief Calculates the lineal interpolation between two numbers given t.
     *
     * @param start
     * @param end
     * @param t
     * @return double
     */
    double lerp(double start, double end, double t);

    /**
     * @brief Calculates the linear interpolation between two known points given
     * a maximum number of points to generate. Doesn't include start and end in
     * the result.
     *
     * @param start
     * @param end
     * @param max_points
     * @return std::vector<Point>
     */
    std::vector<Point> Interpolate(Point start, Point end, int max_points);

    /**
     * @brief Maps a range into another range.
     *
     * @return double
     */
    double map(double x, double in_min, double in_max, double out_min,
               double out_max);

    /**
     * @brief Finds the bounding rect for a set of points.
     *
     * @param points
     * @return Rect
     */
    Rect BoundingRect(const std::vector<Point>& points);

    /**
     * @brief if point is inside the polygon it returns > 0, if outside < 0 else
     * 0 if lies on an edge (or coincides with a vertex)
     *
     * @param points
     * @param point
     * @return double
     */
    double PointPolygonTest(std::vector<Point>& points, Point& point);

    /**
     * @brief Calculates the area of a polygon
     * source: http://alienryderflex.com/polygon_area/
     *
     * @param points
     * @return double
     */
    double PolygonArea(const std::vector<Point>& points);
}  // namespace Observer