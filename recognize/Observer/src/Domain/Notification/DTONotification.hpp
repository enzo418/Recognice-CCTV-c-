#pragma once

#include <string>

#include "../Configuration/NotificationsServiceConfiguration.hpp"

namespace Observer {
    struct DTONotification {
        DTONotification() = default;
        DTONotification(int pGroupID, std::string pContent,
                        ENotificationType pType)
            : groupID(pGroupID), content(pContent), type(pType) {}

        int groupID {-1};
        std::string content {};
        ENotificationType type;
    };

}  // namespace Observer