#include "Notification.hpp"

namespace Web::Domain {
    Notification::Notification(const Observer::DTONotification& notification)
        : content(notification.content), groupID(notification.groupID) {
        this->type = Observer::Helpers::Notifications::NOTIFICATION_TYPE_MAP.at(
            (int)notification.type);

        datetime = std::time_t(0);
    }
}  // namespace Web::Domain