#pragma once

#include <string>
#include <vector>

#include "observer/Domain/Configuration/NotificationsServiceConfiguration.hpp"
#include "observer/Domain/Notification/Notification.hpp"

namespace Observer {
    struct DTONotification {
        DTONotification() = default;
        DTONotification(int pGroupID, const std::string& pContent,
                        ENotificationType pType, const std::string& cameraName,
                        const std::shared_ptr<std::vector<object_detected_t>>&
                            pObjectsDetected)
            : groupID(pGroupID),
              content(pContent),
              type(pType),
              cameraName(cameraName),
              objectsDetected(pObjectsDetected) {}

        int groupID {-1};
        std::string content {};
        ENotificationType type;
        std::string cameraName;
        std::shared_ptr<std::vector<object_detected_t>> objectsDetected;
    };

}  // namespace Observer
