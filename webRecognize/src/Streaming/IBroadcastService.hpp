#pragma once

#include <mutex>

#include "observer/Log/log.hpp"
#include "uWebSockets/App.h"

namespace Web::Streaming {
    /**
     * @brief This is the interface for the broadcast services.
     * Implementations of this interface know how to communicate with the
     * client.
     *
     * @tparam SSL if SSL is enabled
     * @tparam Client Client type, for example uWS::HttpResponse<SSL>
     */
    template <bool SSL, typename Client>
    class IBroadcastService {
       public:
        IBroadcastService() = default;

        virtual void AddClient(Client* res) = 0;

        virtual void RemoveClient(Client* res) = 0;

        virtual int GetTotalClients() = 0;

        virtual void SendToClients(const char* data, int size) = 0;

        virtual void SendToSomeClients(
            const char* data, int size,
            std::function<bool(Client*)> shouldSend) = 0;

       public:
        virtual ~IBroadcastService() = default;
    };
}  // namespace Web::Streaming