#pragma once

#include <string>

#include "Notification.hpp"

namespace Observer {
    class TextNotification : public Notification {
       private:
        std::string text;

       public:
        TextNotification(int groupID, Event ev, std::string text);

        std::string GetCaption() override;
    };

}  // namespace Observer
