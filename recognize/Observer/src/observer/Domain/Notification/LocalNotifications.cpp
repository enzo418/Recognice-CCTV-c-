#include "LocalNotifications.hpp"

#include <curl/curl.h>

#include <utility>

namespace Observer {
    void log_htpp_response(curl_wrapper_response request) {
        if (request.isDone()) {
            OBSERVER_INFO("Notification sended, status code: {}", request.code);
        } else {
            OBSERVER_WARN("Couldn't send the notification, message: {}",
                          request.content);
        }
    }

    LocalNotifications::LocalNotifications(
        const LocalWebNotificationsConfiguration& cfg)
        : MessagingService(cfg) {}

    void LocalNotifications::InternalSendText(
        const DTONotification& notification) {
        this->notificationsPublisher.notifySubscribers(notification);
    }

    void LocalNotifications::InternalSendImage(
        const DTONotification& notification) {
        this->notificationsPublisher.notifySubscribers(notification);
    }

    void LocalNotifications::InternalSendVideo(
        const DTONotification& notification) {
        this->notificationsPublisher.notifySubscribers(notification);
    }

    void LocalNotifications::SubscribeToNewNotifications(
        INotificationEventSubscriber* subscriber) {
        this->notificationsPublisher.subscribe(subscriber);
    }
};  // namespace Observer
