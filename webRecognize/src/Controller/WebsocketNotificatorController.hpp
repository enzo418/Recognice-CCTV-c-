#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

#include "../Domain/Notification.hpp"
#include "../SocketData.hpp"
#include "DTO/DTONotification.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Streaming/WebsocketService.hpp"
#include "nlohmann/json.hpp"
#include "observer/BlockingFIFO.hpp"
#include "observer/Domain/Notification/LocalNotifications.hpp"
#include "observer/Functionality.hpp"
#include "observer/Log/log.hpp"
#include "observer/Semaphore.hpp"

namespace Web {
    template <bool SSL>
    class WebsocketNotificator final
        : public Observer::Functionality,
          public Streaming::WebsocketService<SSL, PerSocketData> {
       public:
        void update(Web::API::DTONotification ev);

       private:
        void InternalStart();

       private:
        Semaphore smpQueue;
        Observer::BlockingFIFO<Web::API::DTONotification> notifications;
    };

    template <bool SSL>
    void WebsocketNotificator<SSL>::InternalStart() {
        while (this->running) {
            if (smpQueue.acquire_timeout<250>()) {
                OBSERVER_ASSERT(notifications.size() > 0,
                                "semaphore logic error");

                // parse all to json
                nlohmann::json json_obj = nlohmann::json::array();

                while (notifications.size() > 0) {
                    json_obj.push_back(notifications.pop_front());
                }

                const std::string json = json_obj.dump();

                this->SendToClients(json.c_str(), json.size());
            }
        }
    }

    template <bool SSL>
    void WebsocketNotificator<SSL>::update(Web::API::DTONotification ev) {
        notifications.push_back(ev);
        smpQueue.release();
    }
}  // namespace Web