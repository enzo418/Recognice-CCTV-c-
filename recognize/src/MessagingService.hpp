#pragma once

#include "Notification.hpp"
#include "ImageNotification.hpp"
#include "VideoNotification.hpp"
#include "TextNotification.hpp"

#include <iostream>

namespace Observer
{
    class MessagingService
    {
    public:
        virtual void SendText(std::string text) = 0;
        virtual void SendImage(std::string path, std::string message) = 0;
        virtual void SendVideo(std::string path, std::string caption) = 0;

        virtual ~MessagingService() = 0;
    };
} // namespace Observer
