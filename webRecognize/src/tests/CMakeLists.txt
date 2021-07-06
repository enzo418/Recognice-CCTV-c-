cmake_minimum_required(VERSION 3.12)

find_package(fmt)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_executable(tests tests.cpp)

target_link_libraries(tests jsoncpp fmt::fmt)

# set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")