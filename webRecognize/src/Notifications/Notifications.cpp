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
}  // namespace Web