#pragma once

#include "../../Utils/SpecialFunctions.hpp"
#include "LocalWebNotifications.hpp"
//#include <restclient-cpp/restclient.h>
#include "../../Log/log.hpp"
#include "../../Utils/CurlWrapper.hpp"

namespace Observer {
    class RestClientLocalWebNotifications : public LocalWebNotifications {
       public:
        explicit RestClientLocalWebNotifications(std::string restServerUrl);

        void SendText(const DTONotification& notification) override;
        void SendImage(const DTONotification& notification) override;
        void SendVideo(const DTONotification& notification) override;

       private:
    };
};  // namespace Observer