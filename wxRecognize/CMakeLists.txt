cmake_minimum_required(VERSION 3.12)
project(wxRecognize VERSION 1.0.0)

find_package(wxWidgets REQUIRED COMPONENTS net core base)
# Opencv package
find_package(OpenCV REQUIRED)

# Curl package
set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 


set(source_dir "${PROJECT_SOURCE_DIR}")

file(GLOB COMMON_SRC "${source_dir}/src/*.cpp")

file(GLOB RECOG_SRC "${source_dir}/../recognize/src/*.cpp")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

set(SOURCES ${COMMON_SRC})

# Print sources
message(${SOURCES})
message(${RECOG_SRC})

include(${wxWidgets_USE_FILE})

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -lX11")

add_executable(wxRecognize ${SOURCES} ${RECOG_SRC})

message(${wxWidgets_LIBRARIES})

# set -l of opencv and curl
target_link_libraries(wxRecognize ${wxWidgets_LIBRARIES} ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp)
