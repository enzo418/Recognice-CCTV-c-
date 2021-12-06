#include <gtest/gtest.h>

#include <cstdlib>
#include <fstream>
#include <opencv4/opencv2/core/types.hpp>

#include "../Observer/src/Domain/VideoBuffer.hpp"

// include all the implementations
#include "../Observer/Implementations/opencv/ImageTransformation.hpp"

using namespace Observer;

const int BUFFER_SIZE = 10;

template <typename T>
class VideoBufferTest : public testing::Test,
                        public testing::WithParamInterface<const char*> {
   protected:
    VideoBufferTest() : buffer(BUFFER_SIZE, BUFFER_SIZE) {}

    VideoBuffer<T> buffer;
};

// decleare all the types that are going to be tested
using TestTypes = ::testing::Types<cv::Mat>;

// Templated suit test
TYPED_TEST_SUITE(VideoBufferTest, TestTypes);

TYPED_TEST(VideoBufferTest, SimpleAdd) {
    std::vector<TypeParam> images(BUFFER_SIZE * 2);

    for (int i = 0; i < BUFFER_SIZE * 2; i++) {
        images[i] = ImageTransformation<TypeParam>::BlackImage();
    }

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

        auto img = ImageTransformation<TypeParam>::BlackImage();

        this->buffer.AddFrame(img);
    }

    auto frames = this->buffer.PopAllFrames();

    // expect size to be eq to BUFFER_SIZE in left + BUFFER_SIZE in right,
    // BUFFER_SIZE * 2
    ASSERT_EQ(frames.size(), BUFFER_SIZE * 2);
}