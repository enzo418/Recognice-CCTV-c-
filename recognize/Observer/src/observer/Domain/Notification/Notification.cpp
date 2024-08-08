#include "Notification.hpp"

namespace Observer {
    Notification::Notification(int pGroupID, EventDescriptor pEvent,
                               std::string pContent,
                               std::shared_ptr<std::vector<object_detected_t>>&
                                   pSimplifiedObjectsDetected)
        : groupID(pGroupID),
          event(std::move(pEvent)),
          content(std::move(pContent)),
          simplifiedObjectsDetected(pSimplifiedObjectsDetected) {}

    int Notification::GetGroupID() { return this->groupID; }

    EventDescriptor& Notification::GetEvent() & { return this->event; }

    std::string& Notification::GetContent() { return this->content; }

    std::shared_ptr<std::vector<object_detected_t>>
    Notification::GetSimplifiedObjectsDetected() {
        return this->simplifiedObjectsDetected;
    }

}  // namespace Observer
