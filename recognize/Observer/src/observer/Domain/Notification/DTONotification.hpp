#pragma once

#include <string>

#include "observer/Domain/Configuration/NotificationsServiceConfiguration.hpp"

namespace Observer {
    struct DTONotification {
        DTONotification() = default;
        DTONotification(int pGroupID, const std::string& pContent,
                        ENotificationType pType, const std::string& cameraName)
            : groupID(pGroupID),
              content(pContent),
              type(pType),
              cameraName(cameraName) {}

        int groupID {-1};
        std::string content {};
        ENotificationType type;
        std::string cameraName;
    };

}  // namespace Observer