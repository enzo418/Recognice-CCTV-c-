#pragma once

#include <chrono>

#include "../../Timer.hpp"
#include "../../Utils/NotificationTypesHelpers.hpp"
#include "../Configuration/NotificationsServiceConfiguration.hpp"
#include "IMessagingService.hpp"

namespace Observer {
    class MessagingService : public IMessagingService {
       public:
        explicit MessagingService(const NotificationsServiceConfiguration& cfg);

       public:
        void SendText(const DTONotification& notification) final;
        void SendImage(const DTONotification& notification) final;
        void SendVideo(const DTONotification& notification) final;

       protected:
        virtual void InternalSendText(const DTONotification& notification) = 0;
        virtual void InternalSendImage(const DTONotification& notification) = 0;
        virtual void InternalSendVideo(const DTONotification& notification) = 0;

       private:
        bool CooldownEnded(int type, int cooldown);

       protected:
        NotificationsServiceConfiguration cfg;
        std::unordered_map<int, Timer<std::chrono::seconds>> timers;
    };

}  // namespace Observer
