cmake_minimum_required(VERSION 3.12)
project(WebRecognize VERSION 1.0.0)

# We don't support Windows or Apple platforms since seasocks doesn't.
if(WIN32)
    message(FATAL_ERROR "${PROJECT_NAME} does not support Windows")
endif()
if(APPLE)
    message(FATAL_ERROR "${PROJECT_NAME} does not support Apple platforms")
endif()

option(WITH_CUDA "Use cuda DNN" ON)

if (WITH_CUDA)
	MESSAGE(STATUS "WITH_CUDA")
	add_definitions(-DWITH_CUDA)
else()
	MESSAGE(STATUS "NO CUDA")
endif()

# Opencv package
find_package(OpenCV REQUIRED)

find_package(fmt)

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

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread -lX11")

add_executable(webRecognize ${SOURCES} ${RECOG_SRC})

# set -l of opencv and curl 
# add -lssl if use ssl with uwebsocket
target_link_libraries(webRecognize ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp fmt::fmt "-l:uSockets.a" "-lz")
