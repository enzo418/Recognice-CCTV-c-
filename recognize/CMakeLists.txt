cmake_minimum_required(VERSION 3.12)
project(Recognize VERSION 1.0.0)

set(CMAKE_DEBUG_POSTFIX "")


enable_testing()
include(GoogleTest)

# Opencv package
find_package(OpenCV REQUIRED)

find_package(spdlog REQUIRED)

# Curl package
set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 

# find_package(yaml-cpp REQUIRED)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/yaml-cpp)
set(YAML_CPP_INSTALL OFF)
include_directories(${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/yaml-cpp/include)

# Json package
# find_package(JSON REQUIRED)

SET(BUILD_OPTIONAL_BINARIES 0 CACHE STRING "Build optional binaries like blob detection. Default is 0.")
SET(USE_OPENCV 1 CACHE STRING "Use opencv as the computer vision and processing library.")

# =========
#  Sources 
# =========

#set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(USE_OPENCV)
  add_compile_definitions(USE_OPENCV)
  add_definitions(-DUSE_OPENCV)
  set(USE_OPENCV true)
endif()

# message(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
# message(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# message(CMAKE_CURRENT_LIST_FILE ${CMAKE_CURRENT_LIST_FILE})
# message(CMAKE_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_DIR})
# message(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/Observer/CMakeLists.txt)

set (CMAKE_CXX_STANDARD 20)

# Print sources
# message(${COMMON_SOURCES})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# Include yaml
include_directories(${YAML_CPP_INCLUDE_DIR})

add_library(recognize_lib ${COMMON_SOURCES})

set(RECOGNIZER_LIBS recognize_lib ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp yaml-cpp spdlog)

# If the cmake file isn't included then use the sample program
if( CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR )
  add_executable(recognize ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Recognizer.cpp)
  # set -l of opencv and curl
  target_link_libraries(recognize ${RECOGNIZER_LIBS})

  if(BUILD_OPTIONAL_BINARIES)
    add_executable(liveBlobDetection ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/LiveBlobDetection.cpp)
    add_executable(videoBlobDetection ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/VideoBlobDetection.cpp)
    add_executable(cameraRecording ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Recording.cpp)
    add_executable(contoursShow ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Contours.cpp)
    add_executable(camerasTest ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Cameras.cpp)

    target_link_libraries(liveBlobDetection ${RECOGNIZER_LIBS})
    target_link_libraries(videoBlobDetection ${RECOGNIZER_LIBS})
    target_link_libraries(cameraRecording ${RECOGNIZER_LIBS})
    target_link_libraries(contoursShow ${RECOGNIZER_LIBS})
    target_link_libraries(camerasTest ${RECOGNIZER_LIBS})
  endif()
endif()

# =====
# tests
# =====
include(${CMAKE_CURRENT_LIST_DIR}/tests/Tests.cmake)

add_executable(
    tests
    "${TESTS_SOURCES}"
)

target_link_libraries(
    tests
    gtest_main
    ${RECOGNIZER_LIBS}
)

gtest_discover_tests(tests)
