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

        void SendText(std::string text) override;
        void SendImage(std::string path, std::string message) override;
        void SendVideo(std::string path, std::string caption) override;

       private:
    };
};  // namespace Observer