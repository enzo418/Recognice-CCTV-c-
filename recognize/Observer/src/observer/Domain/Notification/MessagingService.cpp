#include "MessagingService.hpp"

#include <utility>

namespace Observer {
    MessagingService::MessagingService(
        const NotificationsServiceConfiguration& pCfg)
        : cfg(pCfg) {}

    bool MessagingService::CooldownEnded(int type, int cooldown) {
        auto& timer = timers[type];

        bool ended = true;

        if (!timer.Started()) {
            timer.Start();
        } else {
            if (timer.GetDuration() >= cooldown) {
                timer.GetDurationAndRestart();
            } else {
                ended = false;
            }
        }

        return ended;
    }

    void MessagingService::SendText(const DTONotification& notification) {
        if (this->CooldownEnded(flag_to_int(ENotificationType::TEXT),
                                cfg.secondsBetweenTextNotification)) {
            this->InternalSendText(notification);
        }
    }

    void MessagingService::SendImage(const DTONotification& notification) {
        if (this->CooldownEnded(flag_to_int(ENotificationType::IMAGE),
                                cfg.secondsBetweenImageNotification)) {
            this->InternalSendImage(notification);
        }
    };

    void MessagingService::SendVideo(const DTONotification& notification) {
        if (this->CooldownEnded(flag_to_int(ENotificationType::VIDEO),
                                cfg.secondsBetweenVideoNotification)) {
            this->InternalSendVideo(notification);
        }
    }

}  // namespace Observer
