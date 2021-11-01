#pragma once

#include "RestClientLocalWebNotifications.hpp"

#include <utility>

namespace Observer {
    RestClientLocalWebNotifications::RestClientLocalWebNotifications(std::string pRestServerUrl)
    : LocalWebNotifications(
            std::move(pRestServerUrl)) {

    }

    void RestClientLocalWebNotifications::SendText(std::string text) {
        const std::string url = this->restServerUrl + "/addTextNotification";

        RestClient::Response r = RestClient::post(
                this->restServerUrl + "/addTextNotification",
                "application/json",
                SpecialFunctions::JsonStringGenerator({{"text", text}}));

        if (r.code == 200) {
            // TODO: Log ok
        } else {
            // TODO: Log error r.body
        }
    }

    void RestClientLocalWebNotifications::SendImage(std::string path, std::string text) {
        const std::string url = this->restServerUrl + "/addImageNotification";

        RestClient::Response r = RestClient::post(
                url,
                "application/json",
                SpecialFunctions::JsonStringGenerator({{"text", text}, {"image_path", path}}));

        if (r.code == 200) {
            // TODO: Log ok
        } else {
            // TODO: Log error r.body
        }
    }

    void RestClientLocalWebNotifications::SendVideo(std::string path, std::string text) {
        const std::string url = this->restServerUrl + "/addVideoNotification";

        RestClient::Response r = RestClient::post(
                url,
                "application/json",
                SpecialFunctions::JsonStringGenerator({{"text", text}, {"video_path", path}}));

        if (r.code == 200) {
            // TODO: Log ok
        } else {
            // TODO: Log error r.body
        }
    }
};