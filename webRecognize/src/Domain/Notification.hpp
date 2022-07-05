#pragma once

#include <ctime>
#include <string>

#include "Camera.hpp"
#include "observer/Domain/Notification/DTONotification.hpp"
#include "observer/Utils/NotificationTypesHelpers.hpp"

namespace Web::Domain {
    class Notification {
       public:
        std::string id;
        std::string content;
        int groupID;
        std::string type;
        std::time_t datetime;
        Camera camera;

        // use default operators
        auto operator<=>(const Notification&) const = default;

       public:
        Notification() = default;

        Notification(const Observer::DTONotification& notification
                     /*, Camera camera*/);
    };
}  // namespace Web::Domain