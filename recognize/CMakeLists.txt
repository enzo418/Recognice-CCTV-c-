cmake_minimum_required(VERSION 3.12)
project(Recognize VERSION 1.0.0)

# Opencv package
find_package(OpenCV REQUIRED)

# Curl package
set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 

# Json package
# find_package(JSON REQUIRED)

add_subdirectory(src/external/curly.hpp)

# =========
#  Sources 
# =========

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

set(SOURCES
        sample/sample_main.cpp
        src/Timer.hpp
        src/BaseCameraEvent.hpp
        src/BaseChainOfResponsibility.hpp
        src/BaseEventValidator.hpp
        src/BaseObserverPattern.hpp
        src/external/bitmask_operators.hpp
        src/CameraConfiguration.hpp
        src/SimpleBlockingQueue.hpp

        src/CircularFrameBuffer.cpp
        src/CircularFrameBuffer.hpp
        src/RawCameraEvent.cpp src/RawCameraEvent.hpp

        src/CommonEventValidator.hpp
        src/Configuration.hpp
        src/Event.cpp  src/Event.hpp
        src/EventValidator.cpp src/EventValidator.hpp
        src/FrameDisplay.cpp src/FrameDisplay.hpp
        src/FrameProcessor.cpp src/FrameProcessor.hpp

        src/ImageNotification.cpp src/ImageNotification.hpp
        src/InterfaceFunctionality.hpp
        src/MessagingService.hpp
        src/Notification.cpp src/Notification.hpp
        src/NotificationsServiceConfiguration.hpp
        src/OpencvVideoSource.cpp src/OpencvVideoSource.hpp
        src/OpencvVideoWriter.cpp src/OpencvVideoWriter.hpp
        src/RestClientLocalWebNotifications.cpp src/RestClientLocalWebNotifications.hpp
        src/Semaphore.hpp
        src/SpecialFunctions.hpp src/SpecialFunctions.cpp
        src/TelegramNotifications.cpp src/TelegramNotifications.hpp
        src/TextNotification.cpp src/TextNotification.hpp
        src/ThresholdManager.cpp src/ThresholdManager.hpp
        src/ValidatorBySufficientSamples.cpp src/ValidatorBySufficientSamples.hpp
        src/VideoBuffer.cpp src/VideoBuffer.hpp
        src/VideoNotification.cpp src/VideoNotification.hpp
        src/VideoSource.hpp
        src/VideoWriter.hpp

        src/LocalWebNotifications.cpp src/LocalWebNotifications.hpp
        src/ObserverCentral.cpp src/ObserverCentral.hpp

        src/NotificationsController.cpp src/NotificationsController.hpp

        src/CameraObserver.cpp
        src/CameraObserver.hpp)



# Print sources
message(${SOURCES})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread")

add_executable(recognize ${SOURCES})

# Include curl
include_directories(${CURL_INCLUDE_DIR})

# set -l of opencv and curl
target_link_libraries(recognize ${OpenCV_LIBS} ${CURL_LIBRARIES} jsoncpp curly.hpp)
