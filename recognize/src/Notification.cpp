#include "Notification.hpp"

namespace Observer
{
    Notification::Notification(int pGroupID, Event pEvent) : groupID(pGroupID), event(std::move(pEvent)) {}

    int Notification::GetGroupID() {
        return this->groupID;
    }

    Event& Notification::GetEvent() {
        return this->event;
    }

} // namespace Observer
