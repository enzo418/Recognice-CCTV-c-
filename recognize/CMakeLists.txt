cmake_minimum_required(VERSION 3.12)
project(Recognize VERSION 1.0.0)

# Opencv package
find_package(OpenCV REQUIRED)

find_package(spdlog REQUIRED)

# Curl package
set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 

find_package(yaml-cpp REQUIRED)

# Json package
# find_package(JSON REQUIRED)

# =========
#  Sources 
# =========

#set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

include(Observer/CMakeLists.txt)

# If the cmake file isn't included then use the sample program
if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
        set(SOURCES ${COMMON_SOURCES} Observer/Observer.cpp)
endif()

# Print sources
message(${COMMON_SOURCES})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set (CMAKE_CXX_STANDARD 20)

add_executable(recognize ${SOURCES})

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# Include yaml
include_directories(${YAML_CPP_INCLUDE_DIR})

set(LIBS ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp yaml-cpp spdlog)

# set -l of opencv and curl
target_link_libraries(recognize ${LIBS})

# =====
# tests
# =====
include(tests/Tests.cmake)

enable_testing()

add_executable(
    tests
    "${TESTS_SOURCES}"
    "${COMMON_SOURCES}"
)

target_link_libraries(
    tests
    gtest_main
    ${LIBS}
)

include(GoogleTest)
gtest_discover_tests(tests)
