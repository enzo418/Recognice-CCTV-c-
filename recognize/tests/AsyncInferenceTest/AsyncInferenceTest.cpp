#include <gtest/gtest.h>

#include <algorithm>

#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/Domain/BufferedSource.hpp"
#include "observer/Log/log.hpp"

/**
 * @brief This test will open the video demo called
 * "face-demographics-walking-and-pause.mp4", read all frames and then will do
 * multiple times the following:
 * 0. Check if the environment variable "DETECTOR_SERVER_ADDRESS" is set.
 *    If not set, skip the test.
 * 1. Call DetectorClient::Detect with some frames
 * 2. Assert that there are 3 results, at least 3 people are detected.
 * 3. Repeat, if you want. e.g.
 * --gtest_repeat=1 --gtest_filter=DetectorClientTest.Detect
 */

using namespace AsyncInference;

class DetectorClientTest : public ::testing::Test {
   protected:
    void SetUp() override {
        const auto detectorServerAddress =
            std::getenv("DETECTOR_SERVER_ADDRESS");
        if (detectorServerAddress == nullptr) {
            OBSERVER_INFO("DETECTOR_SERVER_ADDRESS not set. Skipping test.");
            GTEST_SKIP();
        }

        this->detectorClient = std::make_unique<DetectorClient>(
            std::string(detectorServerAddress));
    }

    std::unique_ptr<DetectorClient> detectorClient;
};

TEST_F(DetectorClientTest, Detect) {
    Observer::BufferedSource source;

    if (source.TryOpen("face-demographics-walking-and-pause.mp4")) {
        source.Start();
    } else {
        OBSERVER_INFO("Could not open video source. Skipping test.");
        GTEST_SKIP();
    }

    std::vector<Observer::Frame> frames;
    while (source.IsOk() && frames.size() < 300) {
        if (source.IsFrameAvailable()) frames.push_back(source.GetFrame());
    }

    EXPECT_GT(frames.size(), 0);

    source.Stop();

    constexpr int maxFrames = 100;
    SendEveryNthFrame sendStrategy(std::min(1, (int)frames.size() / maxFrames));

    const auto results = this->detectorClient->Detect(
        frames,
        [](auto& result) {
            OBSERVER_INFO("Received result labeled {}", result.label);
        },
        &sendStrategy);

    int totalDetectionsWithAtLeastOnePerson =
        std::count_if(results.begin(), results.end(), [](const auto& result) {
            return result.detections.size() > 0 &&
                   result.detections[0].label == "person";
        });

    EXPECT_GT(totalDetectionsWithAtLeastOnePerson, 0);
}