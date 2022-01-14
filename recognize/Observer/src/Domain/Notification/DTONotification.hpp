#pragma once

#include <string>

#include "../Configuration/NotificationsServiceConfiguration.hpp"

namespace Observer {
    struct DTONotification {
        DTONotification() = default;
        DTONotification(int pGroupID, std::string pCaption,
                        ENotificationType pType, std::string pMediaPath = "")
            : groupID(pGroupID),
              caption(pCaption),
              type(pType),
              mediaPath(pMediaPath) {}

        int groupID {-1};
        std::string caption {};
        std::string mediaPath {};
        ENotificationType type;
    };

}  // namespace Observer