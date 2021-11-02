#pragma once

#include "LocalWebNotifications.hpp"
#include "SpecialFunctions.hpp"
//#include <restclient-cpp/restclient.h>
#include "external/curly.hpp/curly.hpp"

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
};