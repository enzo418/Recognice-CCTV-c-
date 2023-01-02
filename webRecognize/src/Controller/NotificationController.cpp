#include "NotificationController.hpp"

namespace Web::Controller {
    const std::unordered_map<std::string, const std::string> endpoints = {
        {"notification-stream", "/stream/notification/"},
        {"api-notifications", "/api/notifications/"},
        {"api-notification-buffer", "/api/notifications/:group_id/buffer/"},
        {"ws-notifications", "/notifications"}};
}