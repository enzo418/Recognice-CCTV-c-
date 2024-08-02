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
option(OBSERVER_ENABLE_INSTRUMENTATION "Enable instrumentation in the observer library" OFF)
option(ASAN "Enable AddressSanitizer (ASan)" OFF)
option(UBSAN "Enable UndefinedBehaviorSanitizer (UBSan)" OFF)

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

set(OBSERVER_INCLUDE_DIRS ${DEPENDENCY_INCLUDE_DIR} ${DEPENDENCY_INCLUDE_LIST})

target_include_directories(recognize PUBLIC ${OBSERVER_INCLUDE_DIRS})

target_link_directories(recognize PUBLIC ${DEPENDENCY_LIB_DIR})

# ---------------- DEPENDECIES / LIBRARY --------------- #
add_dependencies(recognize ${DEPENDENCY_LIST})

if(USE_OPENCV)
    target_compile_definitions(recognize PUBLIC USE_OPENCV)
endif()

if(OBSERVER_ENABLE_INSTRUMENTATION)
    target_compile_definitions(recognize PUBLIC OBSERVER_ENABLE_INSTRUMENTATION)
endif()

target_link_libraries(recognize ${DEPENDENCY_LIBS})

target_compile_options(recognize PRIVATE -Wall -Wextra -Wpedantic)

if(ASAN)
  list(APPEND darknet_defines __ASAN__=1)
  if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|GNU|Clang")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
  elseif(MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address")
  else()
    message(SEND_ERROR "AddressSanitizer support requires clang, gcc, or msvc. Try again with -DCMAKE_C_COMPILER.")
  endif()
endif()

if(UBSAN)
  list(APPEND darknet_defines __UBSAN__=1)
  if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|GNU|Clang")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fno-omit-frame-pointer")
  elseif(MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=undefined")
  else()
    message(SEND_ERROR "UndefinedBehaviorSanitizer support requires clang, gcc, or msvc. Try again with -DCMAKE_C_COMPILER.")
  endif()
endif()

if(WARNINGS_AS_ERRORS)
  target_compile_options(recognize PRIVATE -Werror)
endif()

target_compile_definitions(recognize PUBLIC ${OBSERVER_DEPENDENCY_COMPILATION_OPTIONS})

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
        add_executable(AsyncInferenceDemo ${CMAKE_CURRENT_LIST_DIR}/Observer/Samples/AsyncInferenceDemo.cpp)

        target_link_libraries(liveBlobDetection recognize)
        target_link_libraries(videoBlobDetection recognize)
        target_link_libraries(cameraRecording recognize)
        target_link_libraries(contoursShow recognize)
        target_link_libraries(camerasTest recognize)
        target_link_libraries(masksTests recognize)
        target_link_libraries(AsyncInferenceDemo recognize)

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
