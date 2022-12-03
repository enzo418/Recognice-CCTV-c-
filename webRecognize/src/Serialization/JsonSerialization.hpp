#pragma once

#include "../Domain/Notification.hpp"
#include "../Notifications/DTONotification.hpp"
#include "nlohmann/json.hpp"
#include "observer/Domain/Configuration/NLHJSONConfiguration.hpp"

namespace nlohmann {
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::Domain::Camera, id, name, uri);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::DTONotification, id, content,
                                       groupID, type, datetime, cameraID);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::Domain::Notification, id, content,
                                       groupID, type, datetime, camera);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Observer::DTONotification, content,
                                       groupID, type);

}  // namespace nlohmann