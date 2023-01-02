#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>

#include "../Observer/src/observer/Rect.hpp"

using namespace Observer;

void RectIs(Rect& rect, int x, int y, int width, int height) {
    ASSERT_EQ(rect.x, x);
    ASSERT_EQ(rect.y, y);
    ASSERT_EQ(rect.width, width);
    ASSERT_EQ(rect.height, height);
    ASSERT_EQ(rect.tl(), Point(x, y));
    ASSERT_EQ(rect.br(), Point(x + width, y + height));
}

TEST(RectTest, Constructors) {
    int x = 20;
    int y = 20;
    int width = 60;
    int height = 90;
    Rect r0(x, y, width, height);
    Rect r1(x, y, Size(width, height));
    Rect r2(Point(x, y), Point(x + width, y + height));
    Rect r3(Point(x, y), Size(width, height));
    Rect r4(x, y, Size(width, height));

    std::vector<Rect> rects = {r0, r1, r2, r3, r4};

    RectIs(r0, x, y, width, height);
    RectIs(r1, x, y, width, height);
    RectIs(r2, x, y, width, height);
    RectIs(r3, x, y, width, height);
    RectIs(r4, x, y, width, height);
}

TEST(RectTest, Area) {
    Rect r1;
    Rect r2(0, 0, 10, 10);

    ASSERT_EQ(r1.area(), 0);
    ASSERT_EQ(r2.area(), 10 * 10);
}

TEST(RectTest, ShouldNotIntersect) {
    Rect r1(0, 0, 20, 20);
    Rect r2(40, 40, 20, 20);

    Rect intersection = r1.Intersection(r2);

    EXPECT_TRUE(intersection.empty());
    ASSERT_EQ(intersection.area(), 0);
}

TEST(RectTest, ShouldIntersect) {
    /**
     * Ultra high res diagram
     *
     * 0,0-------r1--
     *  |           |
     *  | 10,10--r2-|
     *  |   |  int  |
     *  |---------20,20
     *
     */
    Rect r1(0, 0, 20, 20);
    Rect r2(10, 10, 10, 10);

    Rect intersection = r1.Intersection(r2);

    EXPECT_EQ(intersection.x, 10);
    EXPECT_EQ(intersection.y, 10);
    EXPECT_EQ(intersection.width, 10);
    EXPECT_EQ(intersection.height, 10);
    ASSERT_EQ(intersection.area(), 10 * 10);
    ASSERT_TRUE(r2 == intersection);
}