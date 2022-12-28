#include "TextNotification.hpp"

#include <utility>

namespace Observer {
    TextNotification::TextNotification(int pGroupID, EventDescriptor pEvent,
                                       std::string pText)
        : Notification(pGroupID, std::move(pEvent), std::move(pText)) {}
}  // namespace Observer
