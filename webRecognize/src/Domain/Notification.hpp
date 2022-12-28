#pragma once

#include <ctime>
#include <string>

#include "Camera.hpp"
#include "observer/Domain/Notification/DTONotification.hpp"
#include "observer/Utils/NotificationTypesHelpers.hpp"

namespace Web::Domain {
    class Notification {
       public:
        std::string notificationID;
        std::string content;
        int groupID;
        std::string type;
        std::time_t datetime;
        Camera camera;

        // the configuration that generated this notification
        std::string configurationID;

        // use default operators
        auto operator<=>(const Notification&) const = default;

       public:
        Notification() = default;

        Notification(const Observer::DTONotification& notification
                     /*, Camera camera*/);
    };
}  // namespace Web::Domain