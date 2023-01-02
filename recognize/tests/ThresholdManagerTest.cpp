#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <thread>

#include "../Observer/src/observer/Domain/ThresholdManager.hpp"

using namespace Observer;

TEST(ThresholdManagerTest, Minimum) {
    const int min = 10;
    ThresholdManager mg(min, 1, 1);

    double average = 0;

    for (int i = 0; i < 20; i++) {
        average += i % min;
        mg.Add(i % min);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // add one more to update threshold member
    mg.Add(min);

    average += min;
    average /= 21;

    // the final average is min + average
    double final_average = (double)min + average;
    ASSERT_EQ(mg.GetAverage(), final_average);
}

TEST(ThresholdManagerTest, Update) {
    const int min = 0;
    const int average = 40;
    ThresholdManager mg(min, 1, 1);

    // 1. add some values with average = 40
    for (int i = 0; i < 20; i++) {
        mg.Add(average);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    mg.Add(average);

    ASSERT_EQ(mg.GetAverage(), average);

    // 2. wait 1 second
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 3. add some values
    for (int i = 0; i < 20; i++) {
        mg.Add(average);
    }

    // 4. the average should be still 40
    ASSERT_EQ(mg.GetAverage(), average);
}

TEST(ThresholdManagerTest, IncreaseFactor) {
    const int min = 20;
    const int average = 10;

    // when we add a new value, it will counter the minimum value and
    // leaves us with 20 - 10 = 10 as the average in the manager
    const int final_value = average - min;
    ThresholdManager mg(min, 1, 2);

    // add values < 10
    for (int i = 0; i < 20; i++) {
        mg.Add(final_value);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    mg.Add(final_value);

    ASSERT_EQ(mg.GetAverage(), average * 2);
}