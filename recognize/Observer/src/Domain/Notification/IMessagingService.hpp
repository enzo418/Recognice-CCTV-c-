#pragma once

#include <iostream>

#include "ImageNotification.hpp"
#include "Notification.hpp"
#include "TextNotification.hpp"
#include "VideoNotification.hpp"
namespace Observer {
    class IMessagingService {
       public:
        virtual void SendText(std::string text) = 0;
        virtual void SendImage(std::string path, std::string message) = 0;
        virtual void SendVideo(std::string path, std::string caption) = 0;

        virtual ~IMessagingService() = default;
    };
}  // namespace Observer
