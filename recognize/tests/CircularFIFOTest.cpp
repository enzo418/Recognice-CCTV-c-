#include <gtest/gtest.h>

#include "observer/CircularFIFO.hpp"

using namespace Observer;

TEST(CircularFIFOTest, add) {
    CircularFIFO<std::mutex> buffer(3);
    Frame frame1(Size(1, 1), 1);
    Frame frame2(Size(1, 1), 2);
    Frame frame3(Size(1, 1), 3);

    EXPECT_FALSE(buffer.full());
    EXPECT_TRUE(buffer.empty());

    buffer.add(frame1);
    EXPECT_FALSE(buffer.full());
    EXPECT_FALSE(buffer.empty());

    buffer.add(frame2);
    EXPECT_FALSE(buffer.full());
    EXPECT_FALSE(buffer.empty());

    buffer.add(frame3);
    EXPECT_TRUE(buffer.full());
    EXPECT_FALSE(buffer.empty());

    buffer.add(frame1);
    EXPECT_TRUE(buffer.full());
    EXPECT_FALSE(buffer.empty());
}

TEST(CircularFIFOTest, GetOldestSequential) {
    CircularFIFO<std::mutex> buffer(3);
    Frame frame1(Size(1, 1), 1);
    Frame frame2(Size(1, 1), 2);
    Frame frame3(Size(1, 1), 3);

    buffer.add(frame1);
    Frame oldest1 = buffer.pop();
    buffer.add(frame2);
    Frame oldest2 = buffer.pop();
    buffer.add(frame3);
    Frame oldest3 = buffer.pop();

    EXPECT_EQ(oldest1.GetNumberChannels(), 1);
    EXPECT_EQ(oldest2.GetNumberChannels(), 2);
    EXPECT_EQ(oldest3.GetNumberChannels(), 3);
}

TEST(CircularFIFOTest, GetOldestAccumulated) {
    CircularFIFO<std::mutex> buffer(3);
    Frame frame1(Size(1, 1), 1);
    Frame frame2(Size(1, 1), 2);
    Frame frame3(Size(1, 1), 3);

    buffer.add(frame1);
    buffer.add(frame2);
    buffer.add(frame3);

    Frame oldest1 = buffer.pop();
    Frame oldest2 = buffer.pop();
    Frame oldest3 = buffer.pop();

    EXPECT_EQ(oldest1.GetNumberChannels(), 1);
    EXPECT_EQ(oldest2.GetNumberChannels(), 2);
    EXPECT_EQ(oldest3.GetNumberChannels(), 3);
}

TEST(CircularFIFOTest, GetOldestAccumulatedCircular) {
    CircularFIFO<std::mutex> buffer(3);
    Frame frame1(Size(1, 1), 1);
    Frame frame2(Size(1, 1), 2);
    Frame frame3(Size(1, 1), 3);
    Frame frame4(Size(1, 1), 4);

    buffer.add(frame1);
    buffer.add(frame2);
    buffer.add(frame3);
    buffer.add(frame4);

    Frame oldest1 = buffer.pop();
    Frame oldest2 = buffer.pop();
    Frame oldest3 = buffer.pop();

    EXPECT_EQ(oldest1.GetNumberChannels(), 2);
    EXPECT_EQ(oldest2.GetNumberChannels(), 3);
    EXPECT_EQ(oldest3.GetNumberChannels(), 4);
}