#pragma once

#include <optional>
#include <string>
#include <utility>

#include "../Event/Event.hpp"

namespace Observer {
    /**
     * @brief A notification, it might be a video, image or text.
     */
    class Notification {
       public:
        Notification(int groupID, Event event, std::string content);

        int GetGroupID();

        /**
         * @brief For a video notification, content will be the video path,
         * same for image notification and for text notification it will be
         * the text.
         *
         * @return std::string&
         */
        virtual std::string& GetContent();

        Event& GetEvent() &;

       protected:
        int groupID;
        Event event;
        std::string content;
    };

}  // namespace Observer
