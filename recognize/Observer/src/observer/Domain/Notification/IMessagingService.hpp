#pragma once

#include <iostream>

#include "DTONotification.hpp"
#include "ImageNotification.hpp"
#include "Notification.hpp"
#include "TextNotification.hpp"
#include "VideoNotification.hpp"

namespace Observer {
    class IMessagingService {
       public:
        virtual void SendText(const DTONotification& notification) = 0;
        virtual void SendImage(const DTONotification& notification) = 0;
        virtual void SendVideo(const DTONotification& notification) = 0;

        virtual ~IMessagingService() = default;
    };
}  // namespace Observer
