#pragma once

#include "observer/Log/log.hpp"
#include "uWebSockets/HttpResponse.h"

namespace Web::Streaming::Video::Http {
    template <bool SSL>
    class HttpStreamingService {
       private:
        typedef uWS::HttpResponse<SSL> Client;

       public:
        HttpStreamingService();

       public:
        void AddClient(Client* res);
        void SendToClients(const char* data, int size);
        void RemoveClient(Client* res);

       private:
        typedef std::lock_guard<std::mutex> ScopeLock;
        std::mutex mtxClients;
        std::vector<Client*> clients;
        std::string contentType {"image/jpeg"};
        const char* boundary = "--boundarydonotcross";
    };

    template <bool SSL>
    HttpStreamingService<SSL>::HttpStreamingService() {}

    template <bool SSL>
    void HttpStreamingService<SSL>::AddClient(Client* client) {
        client->onAborted([&] {
            OBSERVER_INFO("Client aborted");
            this->RemoveClient(client);
        });

        this->clients.push_back(client);

        client->writeHeader(
            "Content-Type",
            "multipart/x-mixed-replace;boundary=--boundarydonotcross\r\n"
            "\r\n"
            "----boundarydonotcross\r\n");
    }

    template <bool SSL>
    void HttpStreamingService<SSL>::SendToClients(const char* data, int size) {
        ScopeLock guard_c(this->mtxClients);

        for (auto& client : this->clients) {
            // ref
            // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types#multipartform-data
            OBSERVER_TRACE("Sending intermediate header");

            if (!client->writeRaw("Content-Type: image/jpeg\r\n"
                                  "Content-Length: " +
                                  std::to_string(size) +
                                  "\r\n"
                                  "\r\n"))
                continue;

            OBSERVER_TRACE("Sending image of size {}", size);
            if (!client->writeRaw(data, size)) continue;

            OBSERVER_TRACE("Sending boundary");
            if (!client->writeRaw("\r\n"
                                  "----boundarydonotcross\r\n"))
                continue;
        }
    }

    template <bool SSL>
    void HttpStreamingService<SSL>::RemoveClient(Client* client) {
        ScopeLock guard_c(this->mtxClients);

        auto it = std::find(this->clients.begin(), this->clients.end(), client);

        if (it != this->clients.end()) {
            this->clients.erase(it);
        }
    }
}  // namespace Web::Streaming::Video::Http