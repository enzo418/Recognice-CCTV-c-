#pragma once

#include <string>

#include "Notification.hpp"

namespace Observer {
    class TextNotification : public Notification {
       public:
        TextNotification(int groupID, EventDescriptor ev, std::string text,
                         std::shared_ptr<std::vector<object_detected_t>>&
                             simplifiedObjectsDetected);
    };

}  // namespace Observer
