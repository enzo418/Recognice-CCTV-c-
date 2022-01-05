#pragma once

#include "../../uWebSockets/src/App.h"
#include "DTONotification.hpp"

namespace Web {
    struct NotificationsContext {
        std::string socketTopic;

        std::string textEndpoint;
        std::string imageEndpoint;
        std::string videoEndpoint;
    };
};  // namespace Web

namespace Web {
    void SetNotificationsEndPoints(uWS::App& app, NotificationsContext& ctx);
};