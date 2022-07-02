#pragma once

#include "../../Size.hpp"
#include "CameraConfiguration.hpp"
#include "NotificationsServiceConfiguration.hpp"
#include "OutputPreviewConfiguration.hpp"

namespace Observer {
    struct Configuration {
        struct ResizeNotification {
            // [1, 100], 100 is no resize
            int image;
            int video;

            bool operator==(const ResizeNotification&) const = default;
        };

        // configuration name
        std::string name;

        // absolute path
        std::string mediaFolderPath;

        std::string notificationTextTemplate;

        TelegramNotificationsConfiguration telegramConfiguration;

        LocalWebNotificationsConfiguration localWebConfiguration;

        OutputPreviewConfiguration outputConfiguration;

        std::vector<CameraConfiguration> camerasConfiguration;

        ResizeNotification resizeNotifications;

        bool operator==(const Configuration&) const = default;
    };

}  // namespace Observer
