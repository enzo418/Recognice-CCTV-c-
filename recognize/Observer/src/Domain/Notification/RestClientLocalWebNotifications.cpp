#include "RestClientLocalWebNotifications.hpp"

#include <utility>

namespace Observer {
    void log_htpp_response(curly_hpp::request request) {
        if (request.is_done()) {
            auto response = request.take();
            OBSERVER_INFO("Notification sended, status code: {}",
                          response.http_code());
        } else {
            OBSERVER_WARN("Couldn't send the notification, message: {}",
                          request.get_error());
        }
    }

    RestClientLocalWebNotifications::RestClientLocalWebNotifications(
        std::string pRestServerUrl)
        : LocalWebNotifications(std::move(pRestServerUrl)) {}

    void RestClientLocalWebNotifications::SendText(std::string text) {
        const std::string url = this->restServerUrl + "/addTextNotification";

        curly_hpp::request_builder()
            .method(curly_hpp::http_method::POST)
            .url(url)
            .header("Content-Type", "application/json")
            .content(SpecialFunctions::JsonStringGenerator({{"text", text}}))
            .callback(log_htpp_response)
            .send();
    }

    void RestClientLocalWebNotifications::SendImage(std::string path,
                                                    std::string text) {
        const std::string url = this->restServerUrl + "/addImageNotification";

        curly_hpp::request_builder()
            .method(curly_hpp::http_method::POST)
            .url(url)
            .header("Content-Type", "application/json")
            .content(SpecialFunctions::JsonStringGenerator(
                {{"text", text}, {"image_path", path}}))
            .callback(log_htpp_response)
            .send();
    }

    void RestClientLocalWebNotifications::SendVideo(std::string path,
                                                    std::string text) {
        const std::string url = this->restServerUrl + "/addVideoNotification";
        curly_hpp::request_builder()
            .method(curly_hpp::http_method::POST)
            .url(url)
            .header("Content-Type", "application/json")
            .content(SpecialFunctions::JsonStringGenerator(
                {{"text", text}, {"video_path", path}}))
            .callback(log_htpp_response)
            .send();
    }
};  // namespace Observer