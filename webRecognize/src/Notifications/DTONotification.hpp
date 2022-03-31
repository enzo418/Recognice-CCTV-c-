#pragma once

#include <ctime>
#include <string>
#include <unordered_map>

#include "../../../recognize/Observer/src/Domain/Configuration/Configuration.hpp"
#include "../../../recognize/Observer/src/Domain/Notification/DTONotification.hpp"
#include "../../../recognize/Observer/src/Utils/SpecialEnums.hpp"
#include "../Domain/Notification.hpp"

namespace Web {
    struct DTONotification : Observer::DTONotification {
        DTONotification() = default;

        DTONotification(const Domain::Notification& notf);

        std::string id;
        std::string type;
        std::time_t datetime;
        std::string cameraID;
    };
};  // namespace Web