#pragma once

#include "MessagingService.hpp"
#include "observer/Domain/Configuration/NotificationsServiceConfiguration.hpp"
#include "observer/Log/log.hpp"
#include "observer/Utils/CurlWrapper.hpp"

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
