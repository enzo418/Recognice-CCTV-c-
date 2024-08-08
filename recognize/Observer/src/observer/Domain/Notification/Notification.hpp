#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "observer/Domain/Event/EventDescriptor.hpp"

namespace Observer {
    struct object_detected_t {
        std::string class_name;
        int count;
    };

    /**
     * @brief A notification, it might be a video, image or text.
     */
    class Notification {
       public:
        Notification(int groupID, EventDescriptor event, std::string content,
                     std::shared_ptr<std::vector<object_detected_t>>&
                         simplifiedObjectsDetected);

        int GetGroupID();

        /**
         * @brief For a video notification, content will be the video path,
         * same for image notification and for text notification it will be
         * the text.
         *
         * @return std::string&
         */
        virtual std::string& GetContent();

        EventDescriptor& GetEvent() &;

        std::shared_ptr<std::vector<object_detected_t>>
        GetSimplifiedObjectsDetected();

       protected:
        int groupID;
        EventDescriptor event;
        std::string content;
        std::shared_ptr<std::vector<object_detected_t>>
            simplifiedObjectsDetected;
    };

}  // namespace Observer
