#pragma once

#include "../../Log/log.hpp"
#include "../../Utils/CurlWrapper.hpp"
#include "../Configuration/NotificationsServiceConfiguration.hpp"
#include "IMessagingService.hpp"
namespace Observer {
    class TelegramNotifications : public IMessagingService {
       public:
        TelegramNotifications(TelegramNotificationsConfiguration* cfg);

        void SendText(const DTONotification& notification) override;
        void SendImage(const DTONotification& notification) override;
        void SendVideo(const DTONotification& notification) override;

       private:
        TelegramNotificationsConfiguration* cfg;
        std::string apiEndPoint;
    };

}  // namespace Observer
