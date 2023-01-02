#pragma once

#include <ctime>
#include <string>
#include <unordered_map>

#include "../Domain/Notification.hpp"
#include "observer/Domain/Configuration/Configuration.hpp"
#include "observer/Utils/SpecialEnums.hpp"

namespace Web::API {
    /**
     * @brief Transfer object used to communicate through the API.
     */
    struct DTONotification {
        struct DTONotificationCamera {
            std::string id;
            std::string name;
        };

        DTONotification() = default;

        DTONotification(const Domain::Notification& notification)
            : camera(DTONotificationCamera {.id = notification.camera.cameraID,
                                            .name = notification.camera.name}),
              id(notification.notificationID),
              type(notification.type),
              datetime(notification.datetime),
              groupID(notification.groupID),
              content(notification.content),
              configurationID(notification.configurationID) {}

        std::string id;
        std::time_t datetime;
        int groupID {-1};
        DTONotificationCamera camera;
        std::string type;
        std::string content;

        // the configuration that generated this notification
        std::string configurationID;
    };
};  // namespace Web::API