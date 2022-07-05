#pragma once

#include <ctime>
#include <string>
#include <unordered_map>

#include "../Domain/Notification.hpp"
#include "observer/Domain/Configuration/Configuration.hpp"
#include "observer/Domain/Notification/DTONotification.hpp"
#include "observer/Utils/SpecialEnums.hpp"

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