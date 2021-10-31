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
        TextNotification(int groupID, Event ev, std::string text);

        std::string GetCaption() override;
    };

} // namespace Observer
