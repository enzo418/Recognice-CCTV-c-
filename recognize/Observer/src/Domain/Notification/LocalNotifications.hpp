#pragma once

#include "../../Utils/SpecialFunctions.hpp"
#include "MessagingService.hpp"
//#include <restclient-cpp/restclient.h>
#include "../../Log/log.hpp"
#include "../../Pattern/Event/IEventSubscriber.hpp"
#include "../../Utils/CurlWrapper.hpp"

namespace Observer {
    class INotificationEventSubscriber : public ISubscriber<DTONotification> {
        void update(DTONotification ev) override = 0;
    };
}  // namespace Observer

namespace Observer {
    class LocalNotifications : public MessagingService {
       public:
        explicit LocalNotifications(
            const LocalWebNotificationsConfiguration& cfg);

       public:
        void SubscribeToNewNotifications(
            INotificationEventSubscriber* subscriber);

       protected:
        void InternalSendText(const DTONotification& notification) override;
        void InternalSendImage(const DTONotification& notification) override;
        void InternalSendVideo(const DTONotification& notification) override;

       private:
        std::string restServerUrl;

        Publisher<DTONotification> notificationsPublisher;
    };
};  // namespace Observer