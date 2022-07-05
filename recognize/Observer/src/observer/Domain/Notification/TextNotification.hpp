#pragma once

#include <string>

#include "Notification.hpp"

namespace Observer {
    class TextNotification : public Notification {
       public:
        TextNotification(int groupID, Event ev, std::string text);
    };

}  // namespace Observer
