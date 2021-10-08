#pragma once
#include "MessagingService.hpp"

namespace Observer
{
    class LocalWebNotifications : public MessagingService
    {
    public:
        LocalWebNotifications();
    
        virtual void SendText(std::string text);
        virtual void SendImage(std::string path, std::string message);
        virtual void SendVideo(std::string path, std::string caption);
    private:
    };
    
} // namespace Observer
