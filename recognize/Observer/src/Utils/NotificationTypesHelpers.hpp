#pragma once

#include <unordered_map>

#include "../Domain/Configuration/NotificationsServiceConfiguration.hpp"
#include "SpecialEnums.hpp"

namespace Observer::Helpers::Notifications {
    static const ENotificationType NOTIFICATION_TYPES[] = {
        ENotificationType::TEXT, ENotificationType::IMAGE,
        ENotificationType::VIDEO};

    const std::unordered_map<int, const std::string> NOTIFICATION_TYPE_MAP = {
        {Observer::flag_to_int(Observer::ENotificationType::TEXT), "text"},
        {Observer::flag_to_int(Observer::ENotificationType::IMAGE), "image"},
        {Observer::flag_to_int(Observer::ENotificationType::VIDEO), "video"}};
}  // namespace Observer::Helpers::Notifications