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
        : LocalWebNotifications(std::move(pRestServerUrl)) {}

    void RestClientLocalWebNotifications::SendText(std::string text) {
        const std::string url = this->restServerUrl + "/addTextNotification";

        auto res = CurlWrapper()
                       .url(url)
                       .qparam("text", text)
                       .method(CURLOPT_HTTPGET)
                       .perform();

        log_htpp_response(res);
    }

    void RestClientLocalWebNotifications::SendImage(std::string path,
                                                    std::string text) {
        const std::string url = this->restServerUrl + "/addImageNotification";

        auto res = CurlWrapper()
                       .url(url)
                       .method(CURLOPT_HTTPPOST)
                       .header("Content-Type", "application/json")
                       .body(SpecialFunctions::JsonStringGenerator(
                           {{"text", text}, {"image_path", path}}))
                       .perform();
        log_htpp_response(res);
    }

    void RestClientLocalWebNotifications::SendVideo(std::string path,
                                                    std::string text) {
        const std::string url = this->restServerUrl + "/addVideoNotification";

        auto res = CurlWrapper()
                       .url(url)
                       .method(CURLOPT_HTTPPOST)
                       .header("Content-Type", "application/json")
                       .body(SpecialFunctions::JsonStringGenerator(
                           {{"text", text}, {"video_path", path}}))
                       .perform();
        log_htpp_response(res);
    }
};  // namespace Observer