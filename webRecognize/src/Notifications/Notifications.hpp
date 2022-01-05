#pragma once

#include <fmt/core.h>

#include <string>

#include "DTONotification.hpp"

namespace Web {
    std::string NotificationToJson(const DTONotification& dtoNotification);
}  // namespace Web