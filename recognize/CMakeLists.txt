cmake_minimum_required(VERSION 3.14)
project(Recognize VERSION 1.0.0)

# ----------------------- OPTIONS ---------------------- #
include(CMakeDependentOption)

option(RECO_BUILD_SAMPLES "Build sample executables" OFF)

CMAKE_DEPENDENT_OPTION(
        RECO_BUILD_OPTIONAL_BINARIES 
        "Build optional binaries like blob detection. Default is 0." 
        OFF  # default
        "RECO_BUILD_SAMPLES" # depends on
        OFF # force
)

option(USE_OPENCV "Use opencv as the computer vision and processing library." ON)

option(RECO_TEST "Build tests" OFF)

# disabled by default, compilers change warnings over time
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# -------------- COMPILATION CONFIGURATION ------------- #
set (CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread") # allow threads

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # clangd

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ------------------ MAIN CMAKE FILES ------------------ #

# Dependecies file
include(${CMAKE_CURRENT_LIST_DIR}/Dependency.cmake)

# Sources list
include(${CMAKE_CURRENT_LIST_DIR}/Observer/CMakeLists.txt)

# Add the recognize library
add_library(recognize ${COMMON_SOURCES})

# ------------- INCLUDES / LIBS DIRECTORIES ------------ #
add_include(${CMAKE_CURRENT_LIST_DIR}/Observer/src)

target_include_directories(recognize PUBLIC ${DEPENDENCY_INCLUDE_DIR} ${DEPENDENCY_INCLUDE_LIST})

target_link_directories(recognize PUBLIC ${DEPENDENCY_LIB_DIR})

# ---------------- DEPENDECIES / LIBRARY --------------- #
add_dependencies(recognize ${DEPENDENCY_LIST})

if(USE_OPENCV)
    target_compile_definitions(recognize PUBLIC USE_OPENCV)
endif()

target_link_libraries(recognize ${DEPENDENCY_LIBS})

target_compile_options(recognize PRIVATE -Wall -Wextra -Wpedantic)

if(WARNINGS_AS_ERRORS)
  target_compile_options(recognize PRIVATE -Werror)
endif()

# ---------------------- SAMPLES ---------------------- #
if(RECO_BUILD_SAMPLES)
    add_executable(recognize_sample ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Recognizer.cpp)
    target_link_libraries(recognize_sample recognize)

    if(RECO_BUILD_OPTIONAL_BINARIES)
        add_executable(liveBlobDetection ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/LiveBlobDetection.cpp)
        add_executable(videoBlobDetection ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/VideoBlobDetection.cpp)
        add_executable(cameraRecording ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Recording.cpp)
        add_executable(contoursShow ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Contours.cpp)
        add_executable(camerasTest ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Cameras.cpp)
        add_executable(masksTests ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/Masks.cpp)

        target_link_libraries(liveBlobDetection recognize)
        target_link_libraries(videoBlobDetection recognize)
        target_link_libraries(cameraRecording recognize)
        target_link_libraries(contoursShow recognize)
        target_link_libraries(camerasTest recognize)
        target_link_libraries(masksTests recognize)

        target_compile_options(videoBlobDetection PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

# ------------------------ TESTS ----------------------- #
if(RECO_TEST)
    include(${CMAKE_CURRENT_LIST_DIR}/tests/Tests.cmake)

    enable_testing()

    add_executable(
        tests
        "${TESTS_SOURCES}"
    )

    target_link_libraries(
        tests
        gtest_main
        recognize
    )

    include(GoogleTest)
    gtest_discover_tests(tests)
endif()
