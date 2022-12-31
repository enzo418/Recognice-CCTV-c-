#pragma once

namespace Web {
    struct ServerConfiguration {
        // if true will save high resolution version of the notification video,
        // so you the user can test detection algorithm in the same conditions.
        bool SaveNotificationDebugVideo {true};
    };
}  // namespace Web