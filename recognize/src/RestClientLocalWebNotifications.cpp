#include "RestClientLocalWebNotifications.hpp"

#include <utility>

namespace Observer {
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
            .callback([](curly_hpp::request request) {
                if (request.is_done()) {
                    auto response = request.take();
                    // TODO: Log ok
                    std::cout << "Status code: " << response.http_code()
                              << std::endl;
                } else {
                    // TODO: Log error r.body
                    std::cout << "Error message: " << request.get_error()
                              << std::endl;
                }
            })
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
            .callback([](curly_hpp::request request) {
                if (request.is_done()) {
                    auto response = request.take();
                    // TODO: Log ok
                    std::cout << "Status code: " << response.http_code()
                              << std::endl;
                } else {
                    // TODO: Log error r.body
                    std::cout << "Error message: " << request.get_error()
                              << std::endl;
                }
            })
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
            .callback([](curly_hpp::request request) {
                if (request.is_done()) {
                    auto response = request.take();
                    // TODO: Log ok
                    std::cout << "Status code: " << response.http_code()
                              << std::endl;
                } else {
                    // TODO: Log error r.body
                    std::cout << "Error message: " << request.get_error()
                              << std::endl;
                }
            })
            .send();
    }
};  // namespace Observer