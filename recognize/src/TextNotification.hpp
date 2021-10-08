#pragma once

#include "Notification.hpp"

#include <string>

namespace Observer
{
    class TextNotification : public Notification
    {
    private:
        std::string text;

    public:
        TextNotification(int groupID, Event event, std::string text);

        std::string GetCaption();
    };

} // namespace Observer
