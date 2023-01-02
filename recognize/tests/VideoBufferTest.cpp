#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <opencv4/opencv2/core/types.hpp>

#include "../Observer/src/observer/Domain/VideoBuffer.hpp"

// include all the implementations

using namespace Observer;

const int BUFFER_SIZE = 10;

template <typename T>
class VideoBufferTest : public testing::Test,
                        public testing::WithParamInterface<const char*> {
   protected:
    VideoBufferTest() : buffer(BUFFER_SIZE, BUFFER_SIZE) {}

    VideoBuffer buffer;
};

// decleare all the types that are going to be tested
using TestTypes = ::testing::Types<cv::Mat>;

// Templated suit test
TYPED_TEST_SUITE(VideoBufferTest, TestTypes);

TYPED_TEST(VideoBufferTest, SimpleAdd) {
    std::vector<TypeParam> images(BUFFER_SIZE * 2);

    // for (int i = 0; i < BUFFER_SIZE * 2; i++) {
    //     images[i] = ImageTransformation<TypeParam>::BlackImage();
    // }

    // buffer is idle
    EXPECT_EQ(this->buffer.GetState(), BUFFER_IDLE);

    for (int i = 0; i < BUFFER_SIZE * 2; i++) {
        if (i == BUFFER_SIZE / 2) this->buffer.ChangeWasDetected();

        this->buffer.AddFrame(images[i]);
    }

    // expect the buffer to be full
    EXPECT_EQ(this->buffer.GetState(), BUFFER_READY);

    auto frames = this->buffer.PopAllFrames();

    ASSERT_EQ(frames.size(), BUFFER_SIZE * 2);
}

TYPED_TEST(VideoBufferTest, OverrideAdd) {
    const int addTotal = BUFFER_SIZE * 4;

    for (int i = 0; i < addTotal; i++) {
        if (i == BUFFER_SIZE) this->buffer.ChangeWasDetected();

        // auto img = ImageTransformation<TypeParam>::BlackImage();

        auto frame = Frame(Size(10, 10), 1);
        this->buffer.AddFrame(frame);
    }

    auto frames = this->buffer.PopAllFrames();

    // expect size to be eq to BUFFER_SIZE in left + BUFFER_SIZE in right,
    // BUFFER_SIZE * 2
    ASSERT_EQ(frames.size(), BUFFER_SIZE * 2);
}
/*
TEST(VideoBufferTest, ShouldReturnCorrectDataWithoutOverwrite) {
    VideoBuffer<int> buffer(7, 7);
    std::vector<int> before = {1, 2, 3, 4, 5, 6, 7};
    std::vector<int> after = {8, 9, 10, 11, 12, 13, 14};

    for (auto& frame : before) {
        EXPECT_TRUE(buffer.AddFrame(frame) == BUFFER_IDLE);
    }

    buffer.ChangeWasDetected();

    EXPECT_EQ(buffer.GetState(), BUFFER_WAITING_FILL_UP_AFTER);

    for (int i = 0; i < after.size(); i++) {
        if (i == after.size() - 1) {
            EXPECT_TRUE(buffer.AddFrame(after[i]) == BUFFER_READY);
        } else {
            EXPECT_TRUE(buffer.AddFrame(after[i]) ==
                        BUFFER_WAITING_FILL_UP_AFTER);
        }
    }

    std::vector<int> frames = buffer.PopAllFrames();
    for (int i = 0; i < frames.size(); i++) {
        if (i < before.size()) {
            EXPECT_EQ(frames[i], before[i]);
        } else {
            EXPECT_EQ(frames[i], after[i - before.size()]);
        }
    }
}

TEST(VideoBufferTest, ShouldReturnCorrectDataWithOverwrite) {
    VideoBuffer<int> buffer(7, 7);
    std::vector<int> before = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> realBefore = {4, 5, 6, 7, 8, 9, 10};
    std::vector<int> after = {11, 12, 13, 14, 15, 16, 17};

    for (int i = 0; i < before.size(); i++) {
        EXPECT_TRUE(buffer.AddFrame(before[i]) == BUFFER_IDLE);
    }

    buffer.ChangeWasDetected();

    EXPECT_EQ(buffer.GetState(), BUFFER_WAITING_FILL_UP_AFTER);

    for (int i = 0; i < after.size(); i++) {
        if (i == after.size() - 1) {
            EXPECT_TRUE(buffer.AddFrame(after[i]) == BUFFER_READY);
        } else {
            EXPECT_TRUE(buffer.AddFrame(after[i]) ==
                        BUFFER_WAITING_FILL_UP_AFTER);
        }
    }

    std::vector<int> frames = buffer.PopAllFrames();
    for (int i = 0; i < frames.size(); i++) {
        if (i < realBefore.size()) {
            EXPECT_EQ(frames[i], realBefore[i]);
        } else {
            EXPECT_EQ(frames[i], after[i - realBefore.size()]);
        }
    }
}*/