cmake_minimum_required(VERSION 3.12)
project(Recognize VERSION 1.0.0)

# Opencv package
find_package(OpenCV REQUIRED)

# Curl package
set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 

# Json package
# find_package(JSON REQUIRED)

# =========
#  Sources 
# =========
set(source_dir "${PROJECT_SOURCE_DIR}/src")

file(GLOB COMMON_SRC "${source_dir}/*.cpp")

file(GLOB MAIN_SRC "${PROJECT_SOURCE_DIR}/sample/sample_main.cpp")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

set(SOURCES ${COMMON_SRC} ${MAIN_SRC} src/BaseChainOfResponsibility.hpp src/ValidatorBySufficientSamples.cpp src/ValidatorBySufficientSamples.hpp src/EventValidator.cpp src/EventValidator.hpp src/BaseEventValidator.hpp src/BaseCameraEvent.hpp src/InterfaceFunctionality.hpp src/CommonEventValidator.hpp src/Event.cpp)

# Print sources
message(${SOURCES})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

add_executable(recognize ${SOURCES})

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# set -l of opencv and curl
target_link_libraries(recognize ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp)
