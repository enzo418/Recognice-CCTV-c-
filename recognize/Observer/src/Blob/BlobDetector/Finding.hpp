#pragma once

#include <vector>

#include "../../Point.hpp"
#include "../../Rect.hpp"

namespace Observer {

    class Finding {
       public:
        Finding(Point p);
        Finding(std::vector<Point> points);

        /**
         * @brief Get the shortest distance from the center of this blob to a
         * point.
         *
         * @param p point
         * @return double
         */
        double GetShortestDistance(Point& p);

        /**
         * @brief Add points to the findings
         *
         * @param p
         */
        void AddPoint(Point& p);

        /**
         * @brief Returns a reference to the points of the finding.
         *
         * @return std::vector<Point>
         */
        std::vector<Point>& GetPoints();

        /**
         * @brief Moves the points from the finding. After this call, no
         * operation should be done with this object.
         *
         * @return std::vector<Point>
         */
        std::vector<Point> TakePoints();

        /**
         * @brief Get the bounding rect of the finding
         *
         * @return Rect
         */
        Rect GetBoundingRect();

       private:
        std::vector<Point> points;
        Point tl;
        Point br;
    };

}  // namespace Observer