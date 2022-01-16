#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>

#include "../Observer/src/Utils/Math.hpp"

using namespace Observer;

TEST(MathTest, PointPolygonTest) {
    /**
     * 40,20
     *   ° -
     *   |   -  ° 50,10
     *   -          -
     *    -             - ° 70,15
     *     -            -
     *      -         -
     *       °--------
     *      45,30
     */
    std::vector<Point> poly = {{40, 20}, {40, 30}, {70, 15}, {50, 10}};

    std::vector<std::pair<Point, bool>> pointsTest = {
        {{0, 0}, false},   {{53, 20}, true},
        {{47, 24}, true},  {{60, 21}, false},  // almost
        {{39, 26}, false}, {{41, 12}, false},
    };

    for (auto& [pt, inside] : pointsTest) {
        double res = Observer::PointPolygonTest(poly, pt);
        bool isInside = res > 0;
        EXPECT_TRUE(inside == isInside)
            << "Point " << pt << " expected to be "
            << (inside ? "inside" : "outside") << ". But res is " << res;
    }
}