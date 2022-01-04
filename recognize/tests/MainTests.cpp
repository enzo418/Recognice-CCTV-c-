#include <gtest/gtest.h>

#include "../Observer/src/Log/log.hpp"

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    Observer::LogManager::Initialize();

    return RUN_ALL_TESTS();
}