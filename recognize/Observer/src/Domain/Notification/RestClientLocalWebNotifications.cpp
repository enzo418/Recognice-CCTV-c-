#include "RestClientLocalWebNotifications.hpp"

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

    RestClientLocalWebNotifications::RestClientLocalWebNotifications(
        std::string pRestServerUrl)
        : LocalWebNotifications(pRestServerUrl) {}

    void RestClientLocalWebNotifications::SendText(
        const DTONotification& notification) {
        const std::string url = this->restServerUrl + "/addTextNotification";

        auto res = CurlWrapper()
                       .url(url)
                       .qparam("text", notification.caption)
                       .qparam("group_id", std::to_string(notification.groupID))
                       .method(CURLOPT_HTTPGET)
                       .perform();

        log_htpp_response(res);
    }

    void RestClientLocalWebNotifications::SendImage(
        const DTONotification& notification) {
        const std::string url = this->restServerUrl + "/addImageNotification";

        auto res = CurlWrapper()
                       .url(url)
                       .method(CURLOPT_HTTPGET)
                       .qparam("text", notification.caption)
                       .qparam("image_path", notification.mediaPath)
                       .qparam("group_id", std::to_string(notification.groupID))
                       .perform();
        log_htpp_response(res);
    }

    void RestClientLocalWebNotifications::SendVideo(
        const DTONotification& notification) {
        const std::string url = this->restServerUrl + "/addVideoNotification";

        auto res = CurlWrapper()
                       .url(url)
                       .method(CURLOPT_HTTPGET)
                       .qparam("text", notification.caption)
                       .qparam("video_path", notification.mediaPath)
                       .qparam("group_id", std::to_string(notification.groupID))
                       .perform();
        log_htpp_response(res);
    }
};  // namespace Observer