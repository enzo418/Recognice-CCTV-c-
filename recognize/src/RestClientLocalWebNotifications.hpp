#pragma once

#include "LocalWebNotifications.hpp"
#include "SpecialFunctions.hpp"
#include <restclient-cpp/restclient.h>

namespace Observer {
    class RestClientLocalWebNotifications : public LocalWebNotifications {
        explicit RestClientLocalWebNotifications(std::string restServerUrl);

        void SendText(std::string text) override;
        void SendImage(std::string path, std::string message) override;
        void SendVideo(std::string path, std::string caption) override;
    };
};