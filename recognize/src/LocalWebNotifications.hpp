#pragma once
#include "MessagingService.hpp"

namespace Observer {
    class LocalWebNotifications : public MessagingService {
       public:
        explicit LocalWebNotifications(std::string restServerUrl);

        virtual void SendText(std::string text) = 0;
        virtual void SendImage(std::string path, std::string message) = 0;
        virtual void SendVideo(std::string path, std::string caption) = 0;

       protected:
        std::string restServerUrl;
    };

}  // namespace Observer
