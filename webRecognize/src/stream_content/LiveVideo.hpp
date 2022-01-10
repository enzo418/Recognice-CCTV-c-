#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../../../recognize/Observer/src/IFunctionality.hpp"
#include "../../../recognize/Observer/src/ImageTransformation.hpp"
#include "../../../recognize/Observer/src/Log/log.hpp"
#include "../../../recognize/Observer/src/Pattern/Camera/IFrameSubscriber.hpp"
#include "../../uWebSockets/src/App.h"

struct PerSocketData {
    int id;
};

namespace Web {
    template <typename TFrame, bool SSL>
    class LiveVideo : public IFunctionality, public ISubscriber<TFrame> {
       public:
        typedef uWS::WebSocket<SSL, true, PerSocketData> WebSocketClient;

       public:
        LiveVideo(int id, int fps, int quality);

        /**
         * @brief Locking call.
         *
         */
        void Start() override;

        /**
         * @brief Doesn't lock.
         *
         */
        void Stop() override;

        void AddClient(WebSocketClient* res);

        void RemoveClient(WebSocketClient* res);

        void update(TFrame frame) override;

       public:
        int GetID();

       private:
        void UpdateFrame(TFrame& frame);
        void InternalStart();
        void SendToClients(char* data, int size);

       private:
        bool running;
        double waitMs;
        int quality;
        int id;

       private:
        std::vector<WebSocketClient*> clients;
        std::mutex mtxClients;
        int lastClientId {0};

       private:
        TFrame frame;
        std::mutex mtxFrame;
        bool encoded {false};
        bool imageReady {false};
    };

    template <typename TFrame, bool SSL>
    LiveVideo<TFrame, SSL>::LiveVideo(int pId, int pFps, int pQuality)
        : id(pId), waitMs(1000.0 / (double)pFps), quality(pQuality) {}

    template <typename TFrame, bool SSL>
    int LiveVideo<TFrame, SSL>::GetID() {
        return this->id;
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::Start() {
        this->running = true;
        this->InternalStart();
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::Stop() {
        this->running = false;
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::AddClient(WebSocketClient* ws) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);
        ws->getUserData()->id = ++lastClientId;

        this->clients.push_back(ws);
        OBSERVER_INFO("New Client on live view!");
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::update(TFrame frame) {
        this->UpdateFrame(frame);
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::UpdateFrame(TFrame& pFrame) {
        std::lock_guard<std::mutex> guard_f(this->mtxFrame);
        // OBSERVER_TRACE("Updating image");

        Observer::Size size =
            Observer::ImageTransformation<TFrame>::GetSize(pFrame);
        if (!size.empty()) {
            Observer::ImageTransformation<TFrame>::CopyImage(pFrame,
                                                             this->frame);

            this->imageReady = true;
        }

        this->encoded = false;
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::InternalStart() {
        std::vector<uchar> buffer;

        while (this->running) {
            this->mtxFrame.lock();
            if (!this->encoded && this->imageReady) {
                // OBSERVER_TRACE("Encoding image");
                Observer::ImageTransformation<TFrame>::EncodeImage(
                    ".jpg", this->frame, this->quality, buffer);
                this->encoded = true;
            }
            this->mtxFrame.unlock();

            this->SendToClients((char*)buffer.data(), buffer.size());

            std::this_thread::sleep_for(std::chrono::milliseconds((int)waitMs));
        }
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::SendToClients(char* data, int size) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);

        int clientsCount = this->clients.size();
        for (int i = 0; i < clientsCount; i++) {
            // ¿shoud execute this on a separate thread?
            clients[i]->send(std::string_view(data, size), uWS::OpCode::BINARY,
                             true);
        }
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::RemoveClient(WebSocketClient* ws) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);
        const auto end = clients.end();
        for (auto it = clients.begin(); it != end; ++it) {
            if ((*it)->getUserData()->id == ws->getUserData()->id) {
                clients.erase(it);
                break;
            }
        }
    }

}  // namespace Web