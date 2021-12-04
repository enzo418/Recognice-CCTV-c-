cmake_minimum_required(VERSION 3.12)

set(COMMON_SOURCES
        src/Size.hpp
        src/Rect.hpp
        src/Point.hpp

        src/Timer.hpp
        src/BaseCameraEvent.hpp
        src/BaseChainOfResponsibility.hpp
        src/BaseEventValidator.hpp
        src/BaseObserverPattern.hpp
        src/external/bitmask_operators.hpp
        src/CameraConfiguration.hpp
        src/SimpleBlockingQueue.hpp

        src/CircularFrameBuffer.hpp
        src/RawCameraEvent.hpp

        src/CommonEventValidator.hpp
        src/Configuration.hpp
        src/Event.cpp  src/Event.hpp
        src/EventValidator.hpp
        src/FrameDisplay.hpp
        src/FrameProcessor.hpp

        src/ImageNotification.hpp
        src/IFunctionality.hpp
        src/MessagingService.hpp
        src/Notification.cpp src/Notification.hpp
        src/NotificationsServiceConfiguration.hpp
        src/implementations/opencv/VideoSource.hpp
        src/implementations/opencv/VideoWriter.hpp
        src/RestClientLocalWebNotifications.cpp src/RestClientLocalWebNotifications.hpp
        src/Semaphore.hpp
        src/utils/SpecialFunctions.hpp src/utils/SpecialFunctions.cpp
        src/TelegramNotifications.cpp src/TelegramNotifications.hpp
        src/TextNotification.cpp src/TextNotification.hpp
        src/ThresholdManager.cpp src/ThresholdManager.hpp
        src/ValidatorBySufficientSamples.hpp
        src/VideoBuffer.hpp
        src/VideoNotification.hpp
        src/VideoSource.hpp
        src/VideoWriter.hpp

        src/LocalWebNotifications.cpp src/LocalWebNotifications.hpp
        src/ObserverCentral.hpp

        src/NotificationsController.hpp

        src/CameraObserver.hpp src/ConfigurationParser.cpp src/ConfigurationParser.hpp 
        src/utils/SpecialStrings.cpp src/utils/SpecialStrings.hpp
        src/utils/SpecialEnums.hpp
        
        src/extensions/opencv/extension_cv.hpp src/extensions/opencv/extension_cv.cpp

        src/implementations/opencv/ImageTransformation.hpp
        src/implementations/opencv/ImagePersistence.hpp
        src/implementations/opencv/ImageDisplay.hpp
        
        src/log/managers/LogManagerSPD.hpp src/log/managers/LogManagerSPD.cpp
        src/log/managers/log_constants.hpp
        src/log/log.hpp
)