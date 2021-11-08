#pragma once

#include <string>

// include bitmask operators
// explained in
// https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
#include "external/bitmask_operators.hpp"

namespace Observer
{
    enum class ENotificationType {
        NONE = 0,
        TEXT = 1,
        IMAGE = 2,
        VIDEO = 4
    };

    enum class ETrazable {
        NONE = 0,
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

        bool operator==(const NotificationsServiceConfiguration&) const = default;
    };

    // Subclasses

    struct TelegramNotificationsConfiguration : public NotificationsServiceConfiguration
    {
        std::string apiKey;
        std::string chatID;

        bool operator==(const TelegramNotificationsConfiguration&) const = default;
    };

    struct LocalWebNotificationsConfiguration : public NotificationsServiceConfiguration {
        std::string webServerUrl;

        bool operator==(const LocalWebNotificationsConfiguration&) const = default;
    };

    //////////////////////////////////////////////
    // Enum as bitfield, lets set the operators //
    // src: https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
} // namespace Observer


// enable_bitmask_operators -- true -> enable our custom bitmask for our enums
template<>
struct enable_bitmask_operators<Observer::ETrazable>{
    static constexpr bool enable=true;
};

template<>
struct enable_bitmask_operators<Observer::ENotificationType>{
    static constexpr bool enable=true;
};
