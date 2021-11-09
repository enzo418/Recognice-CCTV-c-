#pragma once

#include <optional>
#include <string>
#include <utility>

#include "Event.hpp"

namespace Observer {
    class Notification {
       public:
        Notification(int groupID, Event event);

        int GetGroupID();

        virtual std::string GetCaption() = 0;

        Event& GetEvent() &;

       protected:
        int groupID;
        Event event;
    };

}  // namespace Observer
