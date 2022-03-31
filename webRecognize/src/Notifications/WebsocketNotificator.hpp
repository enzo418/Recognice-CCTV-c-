#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

#include "../../../recognize/Observer/src/Domain/Notification/LocalNotifications.hpp"
#include "../../../recognize/Observer/src/IFunctionality.hpp"
#include "../../../recognize/Observer/src/Log/log.hpp"
#include "../../vendor/json_dto/json_dto.hpp"
#include "../Domain/Notification.hpp"
#include "../SocketData.hpp"
#include "../WebsocketService.hpp"
#include "DTONotification.hpp"

namespace Web {
    template <bool SSL>
    class WebsocketNotificator : public IFunctionality,
                                 public WebsocketService<SSL, PerSocketData> {
       public:
        WebsocketNotificator();
        ~WebsocketNotificator();

        /**
         * @brief Doesn't lock.
         *
         */
        void Start() override;

        /**
         * @brief Doesn't lock.
         *
         */
        void Stop() override;

        void update(Web::DTONotification ev);

       private:
        void InternalStart();

       private:
        bool running;

       private:
        std::mutex mtxNotifications;
        std::deque<Web::DTONotification> notifications;

       private:
        std::thread thread;
    };

    template <bool SSL>
    WebsocketNotificator<SSL>::WebsocketNotificator() {}

    template <bool SSL>
    WebsocketNotificator<SSL>::~WebsocketNotificator() {
        this->Stop();
    }

    template <bool SSL>
    void WebsocketNotificator<SSL>::Start() {
        OBSERVER_ASSERT(!running, "WebsocketNotificator alredy running!");

        this->running = true;

        thread = std::thread(&WebsocketNotificator<SSL>::InternalStart, this);
    }

    template <bool SSL>
    void WebsocketNotificator<SSL>::Stop() {
        this->running = false;

        if (thread.joinable()) {
            thread.join();
        }
    }

    template <bool SSL>
    void WebsocketNotificator<SSL>::InternalStart() {
        while (this->running) {
            mtxNotifications.lock();

            if (!notifications.empty()) {
                // parse all to json
                auto json = json_dto::to_json<std::deque<Web::DTONotification>>(
                    notifications);

                notifications.clear();
                mtxNotifications.unlock();

                this->SendToClients(json.c_str(), json.size());
            } else {
                mtxNotifications.unlock();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }

    template <bool SSL>
    void WebsocketNotificator<SSL>::update(Web::DTONotification ev) {
        std::lock_guard<std::mutex> g_n(mtxNotifications);
        notifications.push_back(ev);
    }
}  // namespace Web