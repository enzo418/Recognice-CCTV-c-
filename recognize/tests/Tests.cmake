cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(TESTS_SOURCES ${CMAKE_CURRENT_LIST_DIR}/MainTests.cpp
                  ${CMAKE_CURRENT_LIST_DIR}/ConfigurationTest.cpp
                #   ${CMAKE_CURRENT_LIST_DIR}/VideoBufferTest.cpp
                  ${CMAKE_CURRENT_LIST_DIR}/ThresholdManagerTest.cpp
                  ${CMAKE_CURRENT_LIST_DIR}/RectTest.cpp
                  ${CMAKE_CURRENT_LIST_DIR}/MathTest.cpp
                  ${CMAKE_CURRENT_LIST_DIR}/CircularFIFOTest.cpp
                  ${CMAKE_CURRENT_LIST_DIR}/AsyncInferenceTest/AsyncInferenceTest.cpp)

# ----------------- COPY TEST VIDEO ----------------- #
file(COPY ${CMAKE_CURRENT_LIST_DIR}/AsyncInferenceTest/face-demographics-walking-and-pause.mp4
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
