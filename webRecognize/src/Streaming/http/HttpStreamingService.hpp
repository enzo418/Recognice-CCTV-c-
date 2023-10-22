#pragma once

#include "Streaming/IBroadcastService.hpp"
#include "observer/Log/log.hpp"
#include "uWebSockets/HttpResponse.h"

namespace Web::Streaming::Http {
    template <bool SSL>
    class HttpStreamingService final
        : public IBroadcastService<SSL, uWS::HttpResponse<SSL>> {
       private:
        typedef uWS::HttpResponse<SSL> Client;

       public:
        HttpStreamingService(const std::string& contentType = "image/jpeg");

       public:
        void AddClient(Client* res) override;
        void SendToClients(const char* data, int size) override;
        void RemoveClient(Client* res) override;
        int GetTotalClients() override { return this->clients.size(); }
        void SendToSomeClients(
            const char* data, int size,
            std::function<bool(Client*)> shouldSend) override;

       private:
        bool SendData(Client* client, const char* data, int size);

       private:
        typedef std::lock_guard<std::mutex> ScopeLock;
        std::mutex mtxClients;
        std::vector<Client*> clients;
        std::string contentType;
        const char* boundary = "Ba4oevQMz99w0418dcnT";
    };

    template <bool SSL>
    HttpStreamingService<SSL>::HttpStreamingService(
        const std::string& pContentType)
        : contentType(pContentType) {}

    template <bool SSL>
    void HttpStreamingService<SSL>::AddClient(Client* client) {
        std::stringstream ss;
        ss << (const void*)client;
        std::string hexAdd = ss.str();

        OBSERVER_INFO("new Client {}", hexAdd);

        client->onAborted([this, client, hexAdd] {
            OBSERVER_INFO("Client aborted {}", hexAdd);
            this->RemoveClient(client);
        });

        this->clients.push_back(client);

        client->writeHeader("Access-Control-Allow-Origin", "*");

        client->writeHeader(
            "Content-Type",
            "multipart/x-mixed-replace;boundary=Ba4oevQMz99w0418dcnT");

        client->writeHeader("Pragma", "no-cache");

        client->writeHeader("Cache-Control",
                            "no-store, no-cache, must-revalidate, pre-check=0, "
                            "post-check=0, max-age=0");

        client->writeHeader("Expires", "-1");

        client->writeHeader("Connection", "close");

        client->writeHeader("Server", "observer");

        if (!client->writeRaw(
                std::string_view("\r\n--Ba4oevQMz99w0418dcnT\r\n")))
            OBSERVER_WARN("Couldn't send boundary");
    }

    template <bool SSL>
    bool HttpStreamingService<SSL>::SendData(Client* client, const char* data,
                                             int size) {
        if (size == 0) return true;

        // Check if the client's send buffer is full
        // while (client->getBufferedAmount() > 0) {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // }
        if (client->getBackPressure() > 16 * 1024) {
            OBSERVER_WARN("Client buffer is full");
            return false;
        }

        // ref
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types#multipartform-data
        // OBSERVER_TRACE("Sending intermediate header");

        // Construct the data packet with headers and boundary
        std::string packet =
            "Content-Type: image/jpeg\r\n"
            "Content-Length: " +
            std::to_string(size) + "\r\n\r\n" + std::string(data, size) +
            "\r\n--Ba4oevQMz99w0418dcnT\r\n";

        // Write the entire packet to the client
        if (!client->writeRaw(packet)) {
            OBSERVER_WARN("Couldn't send data to client");
            return false;
        }

        return true;
    }

    template <bool SSL>
    void HttpStreamingService<SSL>::SendToClients(const char* data, int size) {
        ScopeLock guard_c(this->mtxClients);

        for (auto& client : this->clients) {
            if (!this->SendData(client, data, size)) {
                OBSERVER_WARN("Couldn't send data to client");
            }
        }
    }

    template <bool SSL>
    void HttpStreamingService<SSL>::RemoveClient(Client* client) {
        ScopeLock guard_c(this->mtxClients);

        auto it = std::find(this->clients.begin(), this->clients.end(), client);

        if (it != this->clients.end()) {
            this->clients.erase(it);
            OBSERVER_INFO("Client removed");
        } else {
            std::stringstream ss;
            ss << (const void*)client;
            std::string hexAdd = ss.str();

            OBSERVER_WARN("Client not found {}", hexAdd);
        }
    }

    template <bool SSL>
    void HttpStreamingService<SSL>::SendToSomeClients(
        const char* data, int size, std::function<bool(Client*)> shouldSend) {
        ScopeLock guard_c(this->mtxClients);

        for (auto& client : this->clients) {
            if (shouldSend(client)) {
                if (!this->SendData(client, data, size)) {
                    OBSERVER_WARN("Couldn't send data to client");
                }
            }
        }
    }
}  // namespace Web::Streaming::Http