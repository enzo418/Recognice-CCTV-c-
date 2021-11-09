#pragma once

#include "CameraConfiguration.hpp"
#include "NotificationsServiceConfiguration.hpp"

namespace Observer {

    struct OutputPreviewConfiguration {
        bool showOutput;
        cv::Size resolution;
        double scaleFactor;
        bool showIgnoredAreas;
        bool showProcessedFrames;

        bool operator==(const OutputPreviewConfiguration&) const = default;
    };

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
