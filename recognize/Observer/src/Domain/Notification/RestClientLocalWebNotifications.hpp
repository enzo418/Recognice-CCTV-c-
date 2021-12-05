#pragma once

#include "../../Utils/SpecialFunctions.hpp"
#include "LocalWebNotifications.hpp"
//#include <restclient-cpp/restclient.h>
#include "../../../vendor/curly.hpp/curly.hpp"
#include "../../Log/log.hpp"

namespace Observer {
    class RestClientLocalWebNotifications : public LocalWebNotifications {
       public:
        explicit RestClientLocalWebNotifications(std::string restServerUrl);

        void SendText(std::string text) override;
        void SendImage(std::string path, std::string message) override;
        void SendVideo(std::string path, std::string caption) override;

       private:
        // hold a separate thread for automatically update async requests
        curly_hpp::performer performer;
    };
};  // namespace Observer