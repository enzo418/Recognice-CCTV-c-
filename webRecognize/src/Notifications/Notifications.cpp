#include "Notifications.hpp"

namespace Web {
    int worst_id = 0;

    std::string NotificationToJson(const DTONotification& dtoNotification) {
        worst_id++;
        return fmt::format(
            "{{\"type\":\"{0}\", \"content\":\"{1}\", \"group\":\"{2}\", "
            "\"date\":\"{3}\", \"directory\":\"{4}\", \"id\": {5}, "
            "\"cameraID\": \"{6}\"}}",
            dtoNotification.type, dtoNotification.caption,
            dtoNotification.groupID, dtoNotification.datetime,
            dtoNotification.mediaPath, worst_id, "123");
    }

    DTONotification ObserverDTONotToWebDTONot(
        const Observer::DTONotification& notf) {
        Web::DTONotification notification;
        notification.type =
            Observer::Helpers::Notifications::NOTIFICATION_TYPE_MAP.at(
                (int)notf.type);

        notification.datetime =
            Observer::SpecialFunctions::GetCurrentTime("%d/%m/%Y %H:%M:%S");

        notification.caption = notf.caption;
        notification.groupID = notf.groupID;
        notification.mediaPath = notf.mediaPath;

        return notification;
    }
}  // namespace Web