#pragma once
#include "IMessagingService.hpp"

namespace Observer {
    class LocalWebNotifications : public IMessagingService {
       public:
        explicit LocalWebNotifications(std::string restServerUrl);

        virtual void SendText(const DTONotification& notification) = 0;
        virtual void SendImage(const DTONotification& notification) = 0;
        virtual void SendVideo(const DTONotification& notification) = 0;

       protected:
        std::string restServerUrl;
    };

}  // namespace Observer
