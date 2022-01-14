#pragma once

#include <fmt/core.h>

#include <string>

#include "../../../recognize/Observer/src/Utils/NotificationTypesHelpers.hpp"
#include "../../../recognize/Observer/src/Utils/SpecialFunctions.hpp"
#include "DTONotification.hpp"

namespace Web {
    std::string NotificationToJson(const DTONotification& dtoNotification);

    DTONotification ObserverDTONotToWebDTONot(
        const Observer::DTONotification& notf);
}  // namespace Web