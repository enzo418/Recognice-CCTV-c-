#pragma once

#include "Event.hpp"

#include <string>
#include <optional>
#include <utility>

namespace Observer
{
    class Notification
    {
    public:
        Notification(int groupID, Event event);

        int GetGroupID();

        virtual std::string GetCaption() = 0;

        Event& GetEvent() &;
    protected:
        int groupID;
        Event event;
    };

} // namespace Observer
