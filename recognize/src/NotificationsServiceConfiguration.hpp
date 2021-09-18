#pragma once

#include <string>

namespace Observer
{
    enum ENotificationType
    {
        TEXT = 1,
        IMAGE = 2,
        VIDEO = 4
    };

    enum ETrazable
    {
        IMAGE = 1,
        VIDEO = 2
    };

    struct NotificationsServiceConfiguration
    {
        bool enabled;
        
        double secondsBetweenTextNotification;
        double secondsBetweenImageNotification;
        double secondsBetweenVideoNotification;

        ENotificationType noticationsToSend;

        bool onNotifSendExtraImageNotfWithAllTheCameras;

        ETrazable drawTraceOfChangeOn;
    };

    // Subclasses

    struct TelegramNotificationsConfiguration : NotificationsServiceConfiguration
    {
        std::string apiKey;
        std::string chatID;
    };

    struct LocalWebNotificationsConfiguration : NotificationsServiceConfiguration { };
    
} // namespace Observer
