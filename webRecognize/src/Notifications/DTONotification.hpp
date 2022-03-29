#pragma once

#include <ctime>
#include <string>
#include <unordered_map>

#include "../../../recognize/Observer/src/Domain/Configuration/Configuration.hpp"
#include "../../../recognize/Observer/src/Domain/Notification/DTONotification.hpp"
#include "../../../recognize/Observer/src/Utils/SpecialEnums.hpp"

namespace Web {
    struct DTONotification : Observer::DTONotification {
        DTONotification() = default;

        std::string type;
        std::time_t datetime;
    };
};  // namespace Web