#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

#include "../Domain/Notification.hpp"
#include "../SocketData.hpp"
#include "../WebsocketService.hpp"
#include "DTO/DTONotification.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "nlohmann/json.hpp"
#include "observer/Domain/Notification/LocalNotifications.hpp"
#include "observer/IFunctionality.hpp"
#include "observer/Log/log.hpp"
#include "observer/Semaphore.hpp"
#include "observer/SimpleBlockingQueue.hpp"

namespace Web {
    template <bool SSL>
    class WebsocketNotificator final
        : public IFunctionality,
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

        void update(Web::API::DTONotification ev);

       private:
        void InternalStart();

       private:
        bool running;

       private:
        Semaphore smpQueue;
        Observer::SimpleBlockingQueue<Web::API::DTONotification> notifications;

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
        OBSERVER_ASSERT(!running, "WebsocketNotificator already running!");

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
            if (!smpQueue.acquire_timeout<250>()) {
                // parse all to json
                nlohmann::json json_obj = nlohmann::json::array();

                while (notifications.size() > 0) {
                    json_obj.push_back(notifications.pop());
                }

                const std::string json = json_obj.dump();

                this->SendToClients(json.c_str(), json.size());
            }
        }
    }

    template <bool SSL>
    void WebsocketNotificator<SSL>::update(Web::API::DTONotification ev) {
        notifications.push(ev);
        smpQueue.release();
    }
}  // namespace Web