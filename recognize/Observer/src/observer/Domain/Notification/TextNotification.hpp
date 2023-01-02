#pragma once

#include <string>

#include "Notification.hpp"

namespace Observer {
    class TextNotification : public Notification {
       public:
        TextNotification(int groupID, EventDescriptor ev, std::string text);
    };

}  // namespace Observer
