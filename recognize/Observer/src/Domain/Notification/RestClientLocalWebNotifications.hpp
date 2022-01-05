#pragma once

#include "../../Utils/SpecialFunctions.hpp"
#include "MessagingService.hpp"
//#include <restclient-cpp/restclient.h>
#include "../../Log/log.hpp"
#include "../../Utils/CurlWrapper.hpp"

namespace Observer {
    class RestClientLocalWebNotifications : public MessagingService {
       public:
        explicit RestClientLocalWebNotifications(
            const LocalWebNotificationsConfiguration& cfg);

       protected:
        void InternalSendText(const DTONotification& notification) override;
        void InternalSendImage(const DTONotification& notification) override;
        void InternalSendVideo(const DTONotification& notification) override;

       private:
        std::string restServerUrl;
    };
};  // namespace Observer