#include "Notification.hpp"

#include "Domain/Camera.hpp"

namespace Web::Domain {
    Notification::Notification(const Observer::DTONotification& notification
                               /*, Camera pCamera*/)
        : content(notification.content),
          groupID(notification.groupID),
          camera(notification.cameraName, "")

    {
        this->type = Observer::Helpers::Notifications::NOTIFICATION_TYPE_MAP.at(
            (int)notification.type);

        datetime = time(0);
    }
}  // namespace Web::Domain