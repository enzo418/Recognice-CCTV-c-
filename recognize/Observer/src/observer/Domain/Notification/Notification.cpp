#include "Notification.hpp"

namespace Observer {
    Notification::Notification(int pGroupID, EventDescriptor pEvent,
                               std::string pContent)
        : groupID(pGroupID),
          event(std::move(pEvent)),
          content(std::move(pContent)) {}

    int Notification::GetGroupID() { return this->groupID; }

    EventDescriptor& Notification::GetEvent() & { return this->event; }

    std::string& Notification::GetContent() { return this->content; }

}  // namespace Observer
