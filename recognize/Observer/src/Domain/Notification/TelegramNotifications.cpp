#include "TelegramNotifications.hpp"

namespace Observer {
    void log_htpp_response(curl_wrapper_response request);

    TelegramNotifications::TelegramNotifications(
        TelegramNotificationsConfiguration* pCfg)
        : cfg(pCfg) {
        this->apiEndPoint = "https://api.telegram.org/bot" + this->cfg->apiKey;
    }

    void TelegramNotifications::SendText(const DTONotification& notification) {
        const std::string url = this->apiEndPoint + "/sendMessage";

        auto res = CurlWrapper()
                       .url(url)
                       .qparam("chat_id", this->cfg->chatID)
                       .qparam("text", notification.caption)
                       .method(CURLOPT_HTTPGET)
                       .perform();

        log_htpp_response(res);
    }

    void TelegramNotifications::SendImage(const DTONotification& notification) {
        const std::string url = this->apiEndPoint + "/sendPhoto";

        auto res =
            CurlWrapper()
                .url(url)
                .method(CURLOPT_HTTPPOST)
                .formAdd("photo", CURLFORM_FILE, notification.mediaPath)
                .formAdd("caption", CURLFORM_COPYCONTENTS, notification.caption)
                .formAdd("chat_id", CURLFORM_COPYCONTENTS, this->cfg->chatID)
                .perform();

        log_htpp_response(res);
    }

    void TelegramNotifications::SendVideo(const DTONotification& notification) {
        const std::string url = this->apiEndPoint + "/sendAnimation";

        auto res =
            CurlWrapper()
                .url(url)
                .method(CURLOPT_HTTPPOST)
                .formAdd("animation", CURLFORM_FILE, notification.mediaPath)
                .formAdd("caption", CURLFORM_COPYCONTENTS, notification.caption)
                .formAdd("chat_id", CURLFORM_COPYCONTENTS, this->cfg->chatID)
                .perform();

        log_htpp_response(res);
    }

}  // namespace Observer
