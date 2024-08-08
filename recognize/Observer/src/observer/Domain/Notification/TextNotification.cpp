#include "TextNotification.hpp"

#include <utility>

namespace Observer {
    TextNotification::TextNotification(
        int pGroupID, EventDescriptor pEvent, std::string pText,
        std::shared_ptr<std::vector<object_detected_t>>&
            simplifiedObjectsDetected)
        : Notification(pGroupID, std::move(pEvent), std::move(pText),
                       simplifiedObjectsDetected) {}
}  // namespace Observer
