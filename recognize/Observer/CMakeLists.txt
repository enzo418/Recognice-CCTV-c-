cmake_minimum_required(VERSION 3.12)

set(SRC_
        vendor/bitmask_operators.hpp

        src/Size.hpp src/Size.cpp
        src/Rect.hpp src/Rect.cpp
        src/Point.hpp src/Point.cpp
        src/Vector.hpp src/Vector.cpp

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
        
        src/CircularFrameBuffer.hpp src/CircularFrameBuffer.cpp
        src/Functionality.hpp src/Functionality.cpp
        src/IFunctionality.hpp
        
        # Interfaces for library dependant impl
        src/IImageDisplay.hpp
        src/IImageIO.hpp
        src/IImageProcessing.hpp
        src/Domain/IVideoSource.hpp
        src/Domain/IVideoWriter.hpp
        
        # Implementation
        src/Implementation.hpp

        # configuration
        src/Domain/Configuration/CameraConfiguration.hpp
        src/Domain/Configuration/Configuration.hpp
        src/Domain/Configuration/NotificationsServiceConfiguration.hpp
        src/Domain/Configuration/ConfigurationParser.cpp src/Domain/Configuration/ConfigurationParser.hpp

        # validators
        src/Domain/Validators/ValidatorHandler.hpp src/Domain/Validators/ValidatorHandler.cpp
        src/Domain/Validators/ValidationResult.hpp src/Domain/Validators/ValidationResult.cpp
        src/Domain/Validators/ValidatorByBlobs.hpp src/Domain/Validators/ValidatorByBlobs.cpp

        # domain
        src/Domain/Event/Event.cpp src/Domain/Event/Event.hpp
        src/Domain/Event/CameraEvent.hpp src/Domain/Event/CameraEvent.cpp
        src/Domain/BufferedSource.hpp src/Domain/BufferedSource.cpp
        src/Domain/EventValidator.hpp src/Domain/EventValidator.cpp
        src/Domain/FrameDisplay.hpp src/Domain/FrameDisplay.cpp
        src/Domain/CamerasFramesBlender.hpp src/Domain/CamerasFramesBlender.cpp
        src/Domain/FrameProcessor.hpp src/Domain/FrameProcessor.cpp
        src/Domain/VideoBuffer.hpp src/Domain/VideoBuffer.cpp
        src/Domain/ThresholdManager.cpp src/Domain/ThresholdManager.hpp
        src/Domain/ObserverCentral.hpp src/Domain/ObserverCentral.cpp
        src/Domain/NotificationsController.hpp src/Domain/NotificationsController.cpp
        src/Domain/CameraObserver.hpp src/Domain/CameraObserver.cpp

        # notifications
        src/Domain/Notification/ImageNotification.hpp src/Domain/Notification/ImageNotification.cpp
        src/Domain/Notification/IMessagingService.hpp
        src/Domain/Notification/Notification.cpp src/Domain/Notification/Notification.hpp
        src/Domain/Notification/TelegramNotifications.cpp src/Domain/Notification/TelegramNotifications.hpp
        src/Domain/Notification/TextNotification.cpp src/Domain/Notification/TextNotification.hpp
        src/Domain/Notification/VideoNotification.hpp src/Domain/Notification/VideoNotification.cpp
        src/Domain/Notification/MessagingService.cpp src/Domain/Notification/MessagingService.hpp
        src/Domain/Notification/LocalNotifications.cpp src/Domain/Notification/LocalNotifications.hpp

        # special functions
        src/Utils/SpecialStrings.cpp src/Utils/SpecialStrings.hpp
        src/Utils/SpecialEnums.hpp
        src/Utils/SpecialFunctions.hpp src/Utils/SpecialFunctions.cpp
        src/Utils/CurlWrapper.cpp src/Utils/CurlWrapper.hpp
        src/Utils/Math.cpp src/Utils/Math.hpp

        # extensions
        Extensions/opencv/extension_cv.hpp Extensions/opencv/extension_cv.cpp

        # implementations
        Implementations/opencv/Implementation.hpp
        Implementations/opencv/Frame.hpp Implementations/opencv/Frame.cpp
        Implementations/opencv/ImageIO.hpp Implementations/opencv/ImageIO.cpp
        Implementations/opencv/ImageDisplay.hpp Implementations/opencv/ImageDisplay.cpp
        Implementations/opencv/VideoSource.hpp Implementations/opencv/VideoSource.cpp
        Implementations/opencv/VideoWriter.hpp Implementations/opencv/VideoWriter.cpp
        Implementations/opencv/ImageProcessing.hpp Implementations/opencv/ImageProcessing.cpp
        Implementations/opencv/ImageDrawBlob.hpp Implementations/opencv/ImageDrawBlob.cpp
        Implementations/opencv/FrameContextualizer.cpp
        Implementations/opencv/TypesConversions.cpp

        # logs
        src/Log/managers/LogManagerSPD.hpp src/Log/managers/LogManagerSPD.cpp
        src/Log/managers/log_constants.hpp
        src/Log/log.hpp

        # blobs
        src/Blob/BlobDetector/Blob.cpp
        src/Blob/BlobDetector/Finding.cpp
        src/Blob/BlobDetector/TrackingBlob.cpp
        src/Blob/FramesProcessor/FrameContextualizer.hpp src/Blob/FramesProcessor/FrameContextualizer.cpp
        src/Blob/Contours/ContoursDetector.hpp src/Blob/Contours/ContoursDetector.cpp
        src/Blob/BlobDetector/BlobDetector.hpp src/Blob/BlobDetector/BlobDetector.cpp
        src/Blob/ImageDrawBlob.hpp
)

# ¿? USE PREMAKE PLS
foreach(source_ IN LISTS SRC_)
    LIST(APPEND COMMON_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${source_}")
endforeach()
