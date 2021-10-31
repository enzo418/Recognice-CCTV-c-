#include "TextNotification.hpp"

#include <utility>

namespace Observer
{
    TextNotification::TextNotification(int pGroupID, Event pEvent, std::string pText)
        : text(std::move(pText)), Notification(pGroupID, std::move(pEvent)) { }

    std::string TextNotification::GetCaption() {
        return this->text;
    }
} // namespace Observer
