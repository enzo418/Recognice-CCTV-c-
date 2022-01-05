#pragma once

#include <string>
#include <unordered_map>

#include "../../../recognize/Observer/src/Domain/Configuration/Configuration.hpp"
#include "../../../recognize/Observer/src/Domain/Notification/DTONotification.hpp"
#include "../../../recognize/Observer/src/Utils/SpecialEnums.hpp"

namespace Web {
    const std::unordered_map<int, const std::string> NOTIFICATIONS_MAP = {
        {Observer::flag_to_int(Observer::ENotificationType::TEXT), "text"},
        {Observer::flag_to_int(Observer::ENotificationType::IMAGE), "image"},
        {Observer::flag_to_int(Observer::ENotificationType::VIDEO), "video"}};

    struct DTONotification : Observer::DTONotification {
        DTONotification() = default;

        std::string type;
        std::string datetime;
    };
};  // namespace Web