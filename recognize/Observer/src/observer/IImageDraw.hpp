#pragma once

#include <iostream>

#include "IFrame.hpp"
#include "observer/ScalarVector.hpp"

namespace Observer {
    class IImageDraw {
       public:
        virtual void FillConvexPoly(Frame& image,
                                    const std::vector<Point>& points,
                                    const ScalarVector& color) = 0;

        /**
         * @brief This function is slower than FillConvexPoly but can draw any
         * kind of polygon.
         *
         * @param image
         * @param points
         * @param color
         */
        virtual void FillAnyPoly(Frame& image, const std::vector<Point>& points,
                                 const ScalarVector& color) = 0;

        virtual void DrawContours(
            std::vector<Frame>& frames,
            const std::vector<std::vector<std::vector<Point>>>& videoContours,
            const ScalarVector& color) = 0;
    };
}  // namespace Observer