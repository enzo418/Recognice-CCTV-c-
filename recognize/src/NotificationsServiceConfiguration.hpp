#pragma once

#include <string>

// include bitmask operators
// explained in
// https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
#include "bitmask_operators.hpp"

namespace Observer
{
    enum class ENotificationType
    {
        TEXT = 1,
        IMAGE = 2,
        VIDEO = 4
    };

    enum class ETrazable
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
