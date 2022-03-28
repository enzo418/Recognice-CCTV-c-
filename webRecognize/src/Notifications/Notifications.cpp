#include "Notifications.hpp"

namespace Web {
    int worst_id = 0;

    std::string NotificationToJson(const DTONotification& dtoNotification) {
        worst_id++;
        return fmt::format(
            "{{\"type\":\"{0}\", \"content\":\"{1}\", \"group\":\"{2}\", "
            "\"date\":\"{3}\", \"id\": {4}, \"cameraID\": \"{5}\"}}",
            dtoNotification.type, dtoNotification.content,
            dtoNotification.groupID, dtoNotification.datetime, worst_id, "123");
    }

    DTONotification ObserverDTONotToWebDTONot(
        const Observer::DTONotification& notf) {
        Web::DTONotification notification;
        notification.type =
            Observer::Helpers::Notifications::NOTIFICATION_TYPE_MAP.at(
                (int)notf.type);

        notification.datetime =
            Observer::SpecialFunctions::GetCurrentTime("%d/%m/%Y %H:%M:%S");

        notification.content = notf.content;
        notification.groupID = notf.groupID;

        return notification;
    }
}  // namespace Web