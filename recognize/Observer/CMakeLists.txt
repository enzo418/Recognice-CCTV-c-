cmake_minimum_required(VERSION 3.12)

set(SRC_
        vendor/bitmask_operators.hpp

        src/Size.hpp
        src/Rect.hpp
        src/Point.hpp

        src/Timer.hpp
        src/Pattern/Camera/ICameraEventSubscriber.hpp
        src/Pattern/Camera/IFrameSubscriber.hpp
        src/Pattern/Camera/IThresholdEventSubscriber.hpp
        src/Pattern/Event/IEventSubscriber.hpp
        src/Pattern/Validation/IValidatorHandler.hpp
        src/Pattern/ChainOfResponsibilityBasics.hpp
        src/Pattern/ObserverBasics.hpp
        src/SimpleBlockingQueue.hpp
        src/Semaphore.hpp

        src/ImageDisplay.hpp
        src/ImagePersistence.hpp
        src/ImageTransformation.hpp

        src/CircularFrameBuffer.hpp
        src/IFunctionality.hpp

        # configuration
        src/Domain/Configuration/CameraConfiguration.hpp
        src/Domain/Configuration/Configuration.hpp
        src/Domain/Configuration/NotificationsServiceConfiguration.hpp
        src/Domain/Configuration/ConfigurationParser.cpp src/Domain/Configuration/ConfigurationParser.hpp

        # validators
        src/Domain/Validators/ValidatorHandler.hpp
        src/Domain/Validators/ValidationResult.hpp
        src/Domain/Validators/ValidatorBySufficientSamples.hpp

        # domain
        src/Domain/Event/Event.cpp src/Domain/Event/Event.hpp
        src/Domain/Event/CameraEvent.hpp
        src/Domain/EventValidator.hpp
        src/Domain/FrameDisplay.hpp
        src/Domain/FrameProcessor.hpp
        src/Domain/VideoBuffer.hpp
        src/Domain/VideoSource.hpp
        src/Domain/VideoWriter.hpp
        src/Domain/ThresholdManager.cpp src/Domain/ThresholdManager.hpp
        src/Domain/ObserverCentral.hpp
        src/Domain/NotificationsController.hpp
        src/Domain/CameraObserver.hpp

        # notifications
        src/Domain/Notification/ImageNotification.hpp
        src/Domain/Notification/IMessagingService.hpp
        src/Domain/Notification/Notification.cpp src/Domain/Notification/Notification.hpp
        src/Domain/Notification/TelegramNotifications.cpp src/Domain/Notification/TelegramNotifications.hpp
        src/Domain/Notification/TextNotification.cpp src/Domain/Notification/TextNotification.hpp
        src/Domain/Notification/VideoNotification.hpp
        src/Domain/Notification/ImageNotification.hpp
        src/Domain/Notification/LocalWebNotifications.cpp src/Domain/Notification/LocalWebNotifications.hpp
        src/Domain/Notification/RestClientLocalWebNotifications.cpp src/Domain/Notification/RestClientLocalWebNotifications.hpp

        # special function
        src/Utils/SpecialStrings.cpp src/Utils/SpecialStrings.hpp
        src/Utils/SpecialEnums.hpp
        src/Utils/SpecialFunctions.hpp src/Utils/SpecialFunctions.cpp

        # extensions
        Extensions/opencv/extension_cv.hpp Extensions/opencv/extension_cv.cpp

        # implementations
        Implementations/opencv/ImageTransformation.hpp
        Implementations/opencv/ImagePersistence.hpp
        Implementations/opencv/ImageDisplay.hpp
        Implementations/opencv/VideoSource.hpp
        Implementations/opencv/VideoWriter.hpp

        # logs
        src/Log/managers/LogManagerSPD.hpp src/Log/managers/LogManagerSPD.cpp
        src/Log/managers/log_constants.hpp
        src/Log/log.hpp
)

# ¿? USE PREMAKE PLS
foreach(source_ IN LISTS SRC_)
    LIST(APPEND COMMON_SOURCES "Observer/${source_}")
endforeach()
