#include "Notifications.hpp"

namespace Web {

    std::string NotificationToJson(const DTONotification& dtoNotification) {
        return fmt::format(
            "{{\"type\":\"{0}\", \"content\":\"{1}\", \"group_id\":\"{2}\", "
            "\"datetime\":\"{3}\", \"directory\":\"{4}\"}}",
            dtoNotification.type, dtoNotification.caption,
            dtoNotification.groupID, dtoNotification.datetime,
            dtoNotification.mediaPath);
    }

    DTONotification ObserverDTONotToWebDTONot(
        const Observer::DTONotification& notf) {
        Web::DTONotification notification;
        notification.type =
            Observer::Helpers::Notifications::NOTIFICATION_TYPE_MAP.at(
                (int)notf.type);

        notification.datetime = Observer::SpecialFunctions::GetCurrentTime();

        notification.caption = notf.caption;
        notification.groupID = notf.groupID;
        notification.mediaPath = notf.mediaPath;

        return notification;
    }
}  // namespace Web