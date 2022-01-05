#pragma once

#include <string>

namespace Observer {
    struct DTONotification {
        DTONotification() = default;
        DTONotification(int pGroupID, std::string pCaption,
                        std::string pMediaPath = "")
            : groupID(pGroupID), caption(pCaption), mediaPath(pMediaPath) {}

        int groupID {-1};
        std::string caption {};
        std::string mediaPath {};
    };

}  // namespace Observer