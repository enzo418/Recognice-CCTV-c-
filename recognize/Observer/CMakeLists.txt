cmake_minimum_required(VERSION 3.12)

set(SRC_
        vendor/bitmask_operators.hpp

        src/observer/Size.hpp src/observer/Size.cpp
        src/observer/Rect.hpp src/observer/Rect.cpp
        src/observer/Point.hpp src/observer/Point.cpp
        src/observer/Vector.hpp src/observer/Vector.cpp
        src/observer/ScalarVector.hpp

        src/observer/Timer.hpp
        src/observer/Pattern/Camera/ICameraEventSubscriber.hpp
        src/observer/Pattern/Camera/IFrameSubscriber.hpp
        src/observer/Pattern/Camera/IThresholdEventSubscriber.hpp
        src/observer/Pattern/Event/IEventSubscriber.hpp
        src/observer/Pattern/Validation/IValidatorHandler.hpp
        src/observer/Pattern/ChainOfResponsibilityBasics.hpp
        src/observer/Pattern/ObserverBasics.hpp
        src/observer/SimpleBlockingQueue.hpp
        src/observer/Semaphore.hpp
        
        src/observer/CircularFrameBuffer.hpp src/observer/CircularFrameBuffer.cpp
        src/observer/Functionality.hpp src/observer/Functionality.cpp
        src/observer/IFunctionality.hpp
        
        src/observer/Domain/SynchronizedIDProvider.hpp src/observer/Domain/SynchronizedIDProvider.cpp

        # Interfaces for library dependant impl
        src/observer/IImageDisplay.hpp
        src/observer/IImageIO.hpp
        src/observer/IImageDraw.hpp
        src/observer/IImageProcessing.hpp
        src/observer/Domain/IVideoSource.hpp
        src/observer/Domain/IVideoWriter.hpp
        
        # Implementation
        src/observer/Implementation.hpp

        # configuration
        src/observer/Domain/Configuration/CameraConfiguration.hpp
        src/observer/Domain/Configuration/Configuration.hpp
        src/observer/Domain/Configuration/NotificationsServiceConfiguration.hpp
        src/observer/Domain/Configuration/ConfigurationParser.cpp src/observer/Domain/Configuration/ConfigurationParser.hpp

        # validators
        src/observer/Domain/Validators/ValidatorHandler.hpp src/observer/Domain/Validators/ValidatorHandler.cpp
        src/observer/Domain/Validators/ValidationResult.hpp src/observer/Domain/Validators/ValidationResult.cpp
        src/observer/Domain/Validators/ValidatorByBlobs.hpp src/observer/Domain/Validators/ValidatorByBlobs.cpp
        src/observer/Domain/Validators/ValidatorByNN.hpp src/observer/Domain/Validators/ValidatorByNN.cpp

        # domain
        src/observer/Domain/Event/EventDescriptor.cpp src/observer/Domain/Event/EventDescriptor.hpp
        src/observer/Domain/Event/CameraEvent.hpp src/observer/Domain/Event/CameraEvent.cpp
        src/observer/Domain/BufferedSource.hpp src/observer/Domain/BufferedSource.cpp
        src/observer/Domain/EventValidator.hpp src/observer/Domain/EventValidator.cpp
        src/observer/Domain/FrameDisplay.hpp src/observer/Domain/FrameDisplay.cpp
        src/observer/Domain/CamerasFramesBlender.hpp src/observer/Domain/CamerasFramesBlender.cpp
        src/observer/Domain/FrameProcessor.hpp src/observer/Domain/FrameProcessor.cpp
        src/observer/Domain/VideoBuffer.hpp src/observer/Domain/VideoBuffer.cpp
        src/observer/Domain/ThresholdManager.cpp src/observer/Domain/ThresholdManager.hpp
        src/observer/Domain/ObserverCentral.hpp src/observer/Domain/ObserverCentral.cpp
        src/observer/Domain/NotificationsController.hpp src/observer/Domain/NotificationsController.cpp
        src/observer/Domain/CameraObserver.hpp src/observer/Domain/CameraObserver.cpp

        # notifications
        src/observer/Domain/Notification/ImageNotification.hpp src/observer/Domain/Notification/ImageNotification.cpp
        src/observer/Domain/Notification/IMessagingService.hpp
        src/observer/Domain/Notification/Notification.cpp src/observer/Domain/Notification/Notification.hpp
        src/observer/Domain/Notification/TelegramNotifications.cpp src/observer/Domain/Notification/TelegramNotifications.hpp
        src/observer/Domain/Notification/TextNotification.cpp src/observer/Domain/Notification/TextNotification.hpp
        src/observer/Domain/Notification/VideoNotification.hpp src/observer/Domain/Notification/VideoNotification.cpp
        src/observer/Domain/Notification/MessagingService.cpp src/observer/Domain/Notification/MessagingService.hpp
        src/observer/Domain/Notification/LocalNotifications.cpp src/observer/Domain/Notification/LocalNotifications.hpp

        # special functions
        src/observer/Utils/SpecialStrings.cpp src/observer/Utils/SpecialStrings.hpp
        src/observer/Utils/SpecialEnums.hpp
        src/observer/Utils/SpecialFunctions.hpp src/observer/Utils/SpecialFunctions.cpp
        src/observer/Utils/CurlWrapper.cpp src/observer/Utils/CurlWrapper.hpp
        src/observer/Utils/Math.cpp src/observer/Utils/Math.hpp

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
        Implementations/opencv/ImageDraw.hpp Implementations/opencv/ImageDraw.cpp

        # logs
        src/observer/Log/managers/LogManagerSPD.hpp src/observer/Log/managers/LogManagerSPD.cpp
        src/observer/Log/managers/log_constants.hpp
        src/observer/Log/log.hpp

        # blobs
        src/observer/Blob/BlobDetector/Blob.cpp
        src/observer/Blob/BlobDetector/Finding.cpp
        src/observer/Blob/BlobDetector/TrackingBlob.cpp
        src/observer/Blob/FramesProcessor/FrameContextualizer.hpp src/observer/Blob/FramesProcessor/FrameContextualizer.cpp
        src/observer/Blob/Contours/ContoursDetector.hpp src/observer/Blob/Contours/ContoursDetector.cpp
        src/observer/Blob/BlobDetector/BlobDetector.hpp src/observer/Blob/BlobDetector/BlobDetector.cpp
        src/observer/Blob/ImageDrawBlob.hpp

        # Async infenrece
        src/observer/AsyncInference/DetectorClient.cpp

        # Classification
        src/observer/Domain/Classification/BlobClassification.cpp src/observer/Domain/Classification/BlobClassification.hpp

        # Instrumentation
        src/observer/Instrumentation/palanteer_impl.cpp
        
)

foreach(source_ IN LISTS SRC_)
    LIST(APPEND COMMON_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${source_}")
endforeach()


# Copy assets folders to build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
