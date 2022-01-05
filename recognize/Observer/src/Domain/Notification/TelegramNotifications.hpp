#pragma once

#include "../../Log/log.hpp"
#include "../../Utils/CurlWrapper.hpp"
#include "../Configuration/NotificationsServiceConfiguration.hpp"
#include "MessagingService.hpp"

namespace Observer {
    class TelegramNotifications : public MessagingService {
       public:
        TelegramNotifications(TelegramNotificationsConfiguration* cfg);

       protected:
        void InternalSendText(const DTONotification& notification) override;
        void InternalSendImage(const DTONotification& notification) override;
        void InternalSendVideo(const DTONotification& notification) override;

       private:
        TelegramNotificationsConfiguration* cfg;
        std::string apiEndPoint;
    };

}  // namespace Observer
