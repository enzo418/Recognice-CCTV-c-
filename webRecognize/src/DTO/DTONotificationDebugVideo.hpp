#pragma once

#include <string>

namespace Web {
    /**
     * Stores information about the video that can be used to debug a
     * notification
     */
    struct DTONotificationDebugVideo {
        std::string id {""};

        // file with the buffer
        std::string filePath;

        // notification group id
        int groupID;

        // if it's present then it was already reclaimed
        std::string videoBufferID;

        int fps;

        // buffer duration
        double duration;

        time_t date_unix;

        std::string camera_id;
    };
}  // namespace Web