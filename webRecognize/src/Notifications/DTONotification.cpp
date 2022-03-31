#include "DTONotification.hpp"

namespace Web {
    DTONotification::DTONotification(const Domain::Notification& notf)
        : cameraID(notf.camera.id),
          id(notf.id),
          type(notf.type),
          datetime(notf.datetime) {
        groupID = notf.groupID;
        content = notf.content;
    }
}  // namespace Web