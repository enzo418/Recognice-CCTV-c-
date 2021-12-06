#pragma once

#include "../../Log/log.hpp"
#include "../../Utils/CurlWrapper.hpp"
#include "../Configuration/NotificationsServiceConfiguration.hpp"
#include "IMessagingService.hpp"
namespace Observer {
    class TelegramNotifications : public IMessagingService {
       public:
        TelegramNotifications(TelegramNotificationsConfiguration* cfg);

        virtual void SendText(std::string text);
        virtual void SendImage(std::string path, std::string message);
        virtual void SendVideo(std::string path, std::string caption);

       private:
        TelegramNotificationsConfiguration* cfg;
        std::string apiEndPoint;
    };

}  // namespace Observer
