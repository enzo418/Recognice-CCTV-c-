#pragma once

#include <string>

namespace Web {

    struct ServerConfiguration {
        // filters use to delete old notifications/videos and keep the fs usage
        // low.
        struct NotificationFilter {
            int deleteIfOlderThanDays {90};
        };

        struct NotificationDebugVideoFilter {
            int keepTotalNotReclaimedBelowMB {300};
        };

        // if true will save high resolution version of the notification video,
        // so you the user can test detection algorithm in the same conditions.
        bool SaveNotificationDebugVideo {true};

        // folder where images and video will be stored. With / at the end
        std::string mediaFolder {"media/"};

        NotificationFilter notificationCleanupFilter;

        NotificationDebugVideoFilter notificationDebugVideoFilter;

        // there are no video buffer filter because the user can delete them
        // through the web interface
        // MediaFilter videoBufferFilter;
    };
}  // namespace Web