#pragma once

#include "CameraConfiguration.hpp"
#include "NotificationsServiceConfiguration.hpp"

namespace Observer
{
    enum EObjectDetectionMethod
    {
        NONE = 1,
        HOG_DESCRIPTOR = 1,
        YOLODNN_V4 = 2
    };

    struct OutputPreviewConfiguration
    {
        bool showOutput;
        cv::Size resolution;
        double scaleFactor;
        bool showIgnoredAreas;
        bool showProcessedFrames;
    };

    struct Configuration
    {
        std::string mediaFolderPath;
        EObjectDetectionMethod objectDetectionMethod;

        TelegramNotificationsConfiguration telegramConfiguration;

        LocalWebNotificationsConfiguration localWebConfiguration;

        OutputPreviewConfiguration outputConfiguration;

        std::vector<CameraConfiguration> camerasConfiguration;
    };

} // namespace Observer
