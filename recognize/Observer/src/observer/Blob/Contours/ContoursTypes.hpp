#pragma once

#include <vector>

#include "observer/Point.hpp"

namespace Observer {
    typedef std::vector<std::vector<Point>> FrameContours;
    typedef std::vector<FrameContours> VideoContours;
}  // namespace Observer