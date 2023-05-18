#pragma once

#include "CameraConfiguration.hpp"
#include "NotificationsServiceConfiguration.hpp"
#include "OutputPreviewConfiguration.hpp"
#include "observer/Size.hpp"

namespace Observer {
    struct Configuration {
        struct ResizeNotification {
            // [1, 100], 100 is no resize
            int image {100};
            int video {100};

            bool operator==(const ResizeNotification&) const = default;
        };

        // configuration name
        std::string name;

        // absolute path
        std::string mediaFolderPath {"./"};

        std::string notificationTextTemplate;

        std::string inferenceServerEndpoint;  // ip + port

        TelegramNotificationsConfiguration telegramConfiguration;

        LocalWebNotificationsConfiguration localWebConfiguration;

        OutputPreviewConfiguration outputConfiguration;

        std::vector<CameraConfiguration> cameras;

        ResizeNotification resizeNotifications;

        bool operator==(const Configuration&) const = default;
    };

}  // namespace Observer
