cmake_minimum_required(VERSION 3.12)
project(Recognice VERSION 1.0.0)

find_package(OpenCV REQUIRED)

set(source_dir "${PROJECT_SOURCE_DIR}/src")

file(GLOB COMMON_SRC "${source_dir}/common/*.cpp")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

if(WIN32 OR MINGW OR MSYS OR CYGWIN)
	file(GLOB PLATFORM_SRC "${source_dir}/windows/*.cpp") 
else()
	file(GLOB PLATFORM_SRC "${source_dir}/unix/*.cpp")
endif()

set(SOURCES ${COMMON_SRC} ${PLATFORM_SRC})

message(${SOURCES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(recognice ${SOURCES})

target_link_libraries(recognice ${OpenCV_LIBS})
