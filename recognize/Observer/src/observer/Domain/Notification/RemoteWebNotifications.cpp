#include "RemoteWebNotifications.hpp"

namespace Observer {
    void log_htpp_response(curl_wrapper_response request);

    inline void replaceSubstring(std::string& str, const std::string& from,
                                 const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    void GenerateParams(const DTONotification& notification, auto& req) {
        std::string content =
            (notification.type == ENotificationType::IMAGE
                 ? "image"
                 : (notification.type == ENotificationType::VIDEO ? "video"
                                                                  : "text"));
        req.qparam("content_type", content)
            .qparam("camera_name", notification.cameraName);

        if (notification.objectsDetected) {
            auto data = notification.objectsDetected.get();
            for (const auto& object : *data) {
                std::string name = object.class_name;
                replaceSubstring(name, " ", "_");
                req.qparam(name, std::to_string(object.count));
            }
        }
    }

    RemoteWebNotifications::RemoteWebNotifications(
        RemoteWebNotificationsConfiguration* pCfg)
        : MessagingService(*pCfg), cfg(pCfg) {}

    void RemoteWebNotifications::InternalSendText(
        const DTONotification& notification) {
        CurlWrapper req;
        req.url(this->cfg->endpointUrl).method(CURLOPT_HTTPGET);

        GenerateParams(notification, req);

        auto res = req.perform();

        log_htpp_response(res);
    }

    void RemoteWebNotifications::InternalSendImage(
        const DTONotification& notification) {
        CurlWrapper req;
        req.url(this->cfg->endpointUrl).method(CURLOPT_HTTPGET);

        GenerateParams(notification, req);

        auto res = req.perform();

        log_htpp_response(res);
    }

    void RemoteWebNotifications::InternalSendVideo(
        const DTONotification& notification) {
        CurlWrapper req;
        req.url(this->cfg->endpointUrl).method(CURLOPT_HTTPGET);

        GenerateParams(notification, req);

        auto res = req.perform();

        log_htpp_response(res);
    }

}  // namespace Observer
