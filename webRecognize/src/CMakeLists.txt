cmake_minimum_required(VERSION 3.12)

set(SRC_ 
    main.cpp # entry

    server_utils.cpp
    Controller/NotificationController.cpp
    
    stream_content/StaticFilesHandler.cpp
    stream_content/FileStreamer.cpp
    stream_content/Range.cpp
    stream_content/FileExtension.cpp
    stream_content/FileReader.cpp

    DAL/InMemory/NotificationRepository.cpp
    DAL/InMemory/CameraRepository.cpp
    DAL/NoLiteDB/VideoBufferRepositoryNLDB.cpp
    DAL/NoLiteDB/NotificationRepositoryNLDB.cpp
    DAL/ConfigurationDAO.cpp
    
    Server/ServerConfigurationProvider.cpp
    
    CL/NotificationCL.cpp
    CL/ConfigurationDAOCache.cpp
    
    Domain/Notification.cpp
    Domain/Camera.cpp
    
    base64.cpp
    CronJobScheduler.cpp
    VideoBufferTasksManager.cpp

    Streaming/Video/CameraOnDemand.cpp
    Streaming/Video/ObserverOnDemand.cpp
    Streaming/Video/SourceOnDemand.cpp
)

if (${WITH_WEBRTC})
    set(SRC_ ${SRC_} 
        Streaming/WebRTC/ObserverBroadcastSource.cpp
        Streaming/WebRTC/GenericBroadcastSource.cpp
        Streaming/WebRTC/ConnectionObserver.cpp
        Streaming/WebRTC/BroadcastWebRTC.cpp
    )
endif()


foreach(source_ IN LISTS SRC_)
    LIST(APPEND WEB_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${source_}")
endforeach()