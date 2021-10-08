#include "TextNotification.hpp"

namespace Observer
{
    TextNotification::TextNotification(int pGroupID, Event pEvent, std::string pText) 
        : text(pText), Notification(pGroupID, pEvent) { }

    std::string TextNotification::GetCaption() {
        return this->text;
    }
} // namespace Observer
