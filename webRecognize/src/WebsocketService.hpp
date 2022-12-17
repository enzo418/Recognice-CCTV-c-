#pragma once

#include <mutex>

#include "observer/Log/log.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/WebSocketProtocol.h"

namespace Web {
    template <bool SSL, typename SocketData>
    class WebsocketService {
       public:
        typedef typename uWS::WebSocket<SSL, true, SocketData> WebSocketClient;

       public:
        WebsocketService() = default;

        virtual void AddClient(WebSocketClient* res);

        virtual void RemoveClient(WebSocketClient* res);

        int GetTotalClients();

       protected:
        virtual void SendToClients(const char* data, int size);

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

        int clientsCount = this->clients.size();
        for (int i = 0; i < clientsCount; i++) {
            // ¿should execute this on a separate thread?
            if (clients[i]->send(std::string_view(data, size),
                                 uWS::OpCode::BINARY,
                                 true) != uWS::WsSendStatus::SUCCESS) {
                // frame was not sent, maybe next will
                OBSERVER_WARN("Websocket video back-pressure!");
            }
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
}  // namespace Web