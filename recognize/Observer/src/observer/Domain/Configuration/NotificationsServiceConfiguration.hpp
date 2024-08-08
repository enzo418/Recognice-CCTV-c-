#pragma once

#include <string>

// include bitmask operators
// explained in
// https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
#include "../../../../vendor/bitmask_operators.hpp"

namespace Observer {
    enum class ENotificationType { NONE = 0, TEXT = 1, IMAGE = 2, VIDEO = 4 };

    enum class ETrazable { NONE = 0, IMAGE = 1, VIDEO = 2 };

    struct NotificationsServiceConfiguration {
        bool enabled {true};

        double secondsBetweenTextNotification {15};
        double secondsBetweenImageNotification {15};
        double secondsBetweenVideoNotification {15};

        ENotificationType notificationsToSend {1 << 0 | 1 << 1 | 1 << 2};

        ETrazable drawTraceOfChangeOn {ETrazable::NONE};

        bool operator==(const NotificationsServiceConfiguration&) const =
            default;
    };

    // Subclasses

    struct TelegramNotificationsConfiguration
        : public NotificationsServiceConfiguration {
        std::string apiKey;
        std::string chatID;

        bool operator==(const TelegramNotificationsConfiguration&) const =
            default;
    };
    
    struct LocalWebNotificationsConfiguration
        : public NotificationsServiceConfiguration {
        bool operator==(const LocalWebNotificationsConfiguration&) const =
            default;
    };

    struct RemoteWebNotificationsConfiguration
        : public NotificationsServiceConfiguration {
        // An endpoint that will receive the notification call.
        // Data will be passed as query parameters:
        // * id=<notification id>
        // * content_type=<image|video|text>
        //   content_uri=<string url pointing to the resource>
        // * camera_name=<camera name>
        //   person=<number of person detected>
        //
        // * means the parameter will always be present 
        std::string endpointUrl;

        bool operator==(const RemoteWebNotificationsConfiguration&) const =
            default;
    };

    //////////////////////////////////////////////
    // Enum as bitfield, lets set the operators //
    // src:
    // https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
}  // namespace Observer

// enable_bitmask_operators -- true -> enable our custom bitmask for our enums
template <>
struct enable_bitmask_operators<Observer::ETrazable> {
    static constexpr bool enable = true;
};

template <>
struct enable_bitmask_operators<Observer::ENotificationType> {
    static constexpr bool enable = true;
};
