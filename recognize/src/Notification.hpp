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

        std::string GetCaption();

        Event &GetEvent();

    protected:
        int groupID;
        Event event;
    };

} // namespace Observer
