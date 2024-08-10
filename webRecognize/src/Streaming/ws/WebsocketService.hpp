#pragma once

#include <mutex>

#include "Streaming/IBroadcastService.hpp"
#include "observer/Log/log.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/Loop.h"
#include "uWebSockets/WebSocket.h"
#include "uWebSockets/WebSocketProtocol.h"

namespace Web::Streaming::Ws {
    template <bool SSL, typename SocketData>
    class WebsocketService
        : public IBroadcastService<SSL, uWS::WebSocket<SSL, true, SocketData>> {
       public:
        typedef uWS::WebSocket<SSL, true, SocketData> WebSocketClient;

       public:
        WebsocketService() = default;
        virtual ~WebsocketService() = default;

        virtual void AddClient(WebSocketClient* res) override;

        virtual void RemoveClient(WebSocketClient* res) override;

        int GetTotalClients() override;

        virtual void SendToClients(const char* data, int size) override;

        void SendToSomeClients(
            const char* data, int size,
            std::function<bool(WebSocketClient*)> shouldSend) override;

       private:
        std::vector<WebSocketClient*> clients;
        std::mutex mtxClients;
        int lastClientId {0};
    };

    template <bool SSL, typename SocketData>
    void WebsocketService<SSL, SocketData>::AddClient(WebSocketClient* ws) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);
        ws->getUserData()->id = ++lastClientId;

        this->clients.push_back(ws);
        OBSERVER_INFO("New Client!");
    }

    template <bool SSL, typename SocketData>
    void WebsocketService<SSL, SocketData>::SendToClients(const char* data,
                                                          int size) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);

        if (size == 0) {
            OBSERVER_WARN(
                "Trying to send a 0 size packet. This would cause a TCP "
                "error!!");
            return;
        }

        auto buffer = std::shared_ptr<char[]>(new char[size],
                                              std::default_delete<char[]>());
        std::copy(data, data + size, buffer.get());

        int clientsCount = this->clients.size();
        for (int i = 0; i < clientsCount; i++) {
            /**
             * NOTE: I should be smart and follow the guideline, use
             * getBufferedAmount and drain
             */
            uWS::Loop::get()->defer([buffer, size, client = clients[i]]() {
                auto code = client->send(std::string_view(buffer.get(), size),
                                         uWS::OpCode::BINARY, true);
                if (code != uWS::WsSendStatus::SUCCESS) {
                    if (code == uWS::WsSendStatus::BACKPRESSURE) {
                        OBSERVER_ERROR("Websocket backpressure. Buffered {}",
                                       client->getBufferedAmount());
                    }
                }
            });
        }
    }

    template <bool SSL, typename SocketData>
    void WebsocketService<SSL, SocketData>::SendToSomeClients(
        const char* data, int size,
        std::function<bool(WebSocketClient*)> shouldSend) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);

        if (size == 0) {
            OBSERVER_WARN(
                "Trying to send a 0 size packet. This would cause a TCP "
                "error!!");
            return;
        }

        auto buffer = std::shared_ptr<char[]>(new char[size],
                                              std::default_delete<char[]>());
        std::copy(data, data + size, buffer.get());

        int clientsCount = this->clients.size();
        for (int i = 0; i < clientsCount; i++) {
            /**
             * NOTE: I should be smart and follow the guideline, use
             * getBufferedAmount and drain
             */
            uWS::Loop::get()->defer([shouldSend, buffer, size,
                                     client = clients[i]]() {
                if (!shouldSend(client)) return;

                auto code = client->send(std::string_view(buffer.get(), size),
                                         uWS::OpCode::BINARY, true);
                if (code != uWS::WsSendStatus::SUCCESS) {
                    if (code == uWS::WsSendStatus::BACKPRESSURE) {
                        OBSERVER_ERROR("Websocket backpressure. Buffered {}",
                                       client->getBufferedAmount());
                    }
                }
            });
        }
    }

    template <bool SSL, typename SocketData>
    void WebsocketService<SSL, SocketData>::RemoveClient(WebSocketClient* ws) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);
        const auto end = clients.end();
        for (auto it = clients.begin(); it != end; ++it) {
            if ((*it)->getUserData()->id == ws->getUserData()->id) {
                clients.erase(it);
                break;
            }
        }
    }

    template <bool SSL, typename SocketData>
    int WebsocketService<SSL, SocketData>::GetTotalClients() {
        std::lock_guard<std::mutex> g_clients(mtxClients);

        return clients.size();
    }
}  // namespace Web::Streaming::Ws