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

set (CMAKE_CXX_STANDARD 20)

# Print sources
message(${COMMON_SOURCES})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# Include yaml
include_directories(${YAML_CPP_INCLUDE_DIR})

add_library(recognize_lib ${COMMON_SOURCES})

set(LIBS recognize_lib ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp yaml-cpp spdlog)

# If the cmake file isn't included then use the sample program
if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR )
  add_executable(recognize Observer/Samples/Recognizer.cpp)
  add_executable(liveBlobDetection Observer/Samples/LiveBlobDetection.cpp)
  add_executable(videoBlobDetection Observer/Samples/VideoBlobDetection.cpp)
  add_executable(cameraRecording Observer/Samples/Recording.cpp)

  # set -l of opencv and curl
  target_link_libraries(recognize ${LIBS})
  target_link_libraries(liveBlobDetection ${LIBS})
  target_link_libraries(videoBlobDetection ${LIBS})
  target_link_libraries(cameraRecording ${LIBS})
endif()

# =====
# tests
# =====
include(tests/Tests.cmake)

enable_testing()

add_executable(
    tests
    "${TESTS_SOURCES}"
)

target_link_libraries(
    tests
    gtest_main
    ${LIBS}
)

include(GoogleTest)
gtest_discover_tests(tests)
