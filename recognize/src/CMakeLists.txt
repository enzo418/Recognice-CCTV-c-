cmake_minimum_required(VERSION 3.12)

set(COMMON_SOURCES
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
        src/utils/SpecialFunctions.hpp src/utils/SpecialFunctions.cpp
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
        src/CameraObserver.hpp src/ConfigurationParser.cpp src/ConfigurationParser.hpp 
        src/utils/SpecialStrings.cpp src/utils/SpecialStrings.hpp
        src/utils/SpecialEnums.hpp
        
        src/extensions/opencv/extension_cv.hpp src/extensions/opencv/extension_cv.cpp)