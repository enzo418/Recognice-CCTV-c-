#pragma once

#include "../../Size.hpp"
#include "CameraConfiguration.hpp"
#include "NotificationsServiceConfiguration.hpp"
#include "OutputPreviewConfiguration.hpp"

namespace Observer {
    struct Configuration {
        // absolute path
        std::string mediaFolderPath;

        std::string notificationTextTemplate;

        TelegramNotificationsConfiguration telegramConfiguration;

        LocalWebNotificationsConfiguration localWebConfiguration;

        OutputPreviewConfiguration outputConfiguration;

        std::vector<CameraConfiguration> camerasConfiguration;

        bool operator==(const Configuration&) const = default;
    };

}  // namespace Observer
