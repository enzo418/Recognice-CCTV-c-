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
    };
}  // namespace Observer