#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../../../recognize/Observer/src/IFunctionality.hpp"
#include "../../../recognize/Observer/src/ImageTransformation.hpp"
#include "../../../recognize/Observer/src/Log/log.hpp"
#include "../../../recognize/Observer/src/Pattern/Camera/IFrameSubscriber.hpp"
#include "../../../recognize/Observer/src/Utils/SpecialEnums.hpp"
#include "../../../recognize/Observer/vendor/bitmask_operators.hpp"
#include "../../uWebSockets/src/App.h"
#include "../SocketData.hpp"

namespace Web {
    enum class LiveViewStatus {
        OPEN = 1,
        CLOSED = 2,
        RUNNING = 4,
        STOPPED = 8,
        ERROR = 16
    };
}  // namespace Web

// enable_bitmask_operators -- true -> enable our custom bitmask for our
// enums
template <>
struct enable_bitmask_operators<Web::LiveViewStatus> {
    static constexpr bool enable = true;
};

namespace Web {
    template <typename TFrame, bool SSL>
    class LiveVideo : public IFunctionality {
       public:
        typedef uWS::WebSocket<SSL, true, PerSocketData> WebSocketClient;

       public:
        LiveVideo(int fps, int quality);
        ~LiveVideo();

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

        void AddClient(WebSocketClient* res);

        void RemoveClient(WebSocketClient* res);

        LiveViewStatus GetStatus();

       public:
        int GetTotalClients();

       private:
        void InternalStart();
        void SendToClients(char* data, int size);

       protected:
        void NewValidFrameReceived();

        /**
         * @brief Updates this->frames
         */
        virtual void GetNextFrame() = 0;

        virtual void PostStop();
        virtual void PreStart();

        void SetFPS(double fps);

        void SetQuality(int quality);

       protected:
        TFrame frame;
        std::mutex mtxFrame;
        LiveViewStatus status;

       private:
        bool encoded {false};
        bool imageReady {false};

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
        std::thread thread;
    };

    template <typename TFrame, bool SSL>
    LiveVideo<TFrame, SSL>::LiveVideo(int pFps, int pQuality)
        : waitMs(1000.0 / (double)pFps), quality(pQuality) {
        Observer::set_flag(status, LiveViewStatus::CLOSED);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <typename TFrame, bool SSL>
    LiveVideo<TFrame, SSL>::~LiveVideo() {
        this->Stop();
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::Start() {
        OBSERVER_ASSERT(!running, "Live view alredy runnin!");

        this->running = true;

        thread = std::thread(&LiveVideo<TFrame, SSL>::InternalStart, this);
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::Stop() {
        this->running = false;

        if (thread.joinable()) {
            thread.join();
        }

        this->PostStop();
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::AddClient(WebSocketClient* ws) {
        std::lock_guard<std::mutex> guard_c(this->mtxClients);
        ws->getUserData()->id = ++lastClientId;

        this->clients.push_back(ws);
        OBSERVER_INFO("New Client on live view!");
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::NewValidFrameReceived() {
        this->imageReady = true;
        this->encoded = false;
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::InternalStart() {
        std::vector<uchar> buffer;
        this->PreStart();

        if (Observer::has_flag(status, LiveViewStatus::STOPPED)) {
            Observer::clear_flag(status, LiveViewStatus::STOPPED);
        }

        Observer::set_flag(status, LiveViewStatus::RUNNING);

        while (this->running) {
            this->GetNextFrame();

            this->mtxFrame.lock();
            if (!this->encoded && this->imageReady) {
                Observer::ImageTransformation<TFrame>::EncodeImage(
                    ".jpg", this->frame, this->quality, buffer);
                this->encoded = true;
            }
            this->mtxFrame.unlock();

            this->SendToClients((char*)buffer.data(), buffer.size());

            std::this_thread::sleep_for(std::chrono::milliseconds((int)waitMs));
        }

        Observer::clear_flag(status, LiveViewStatus::RUNNING);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
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

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::SetFPS(double fps) {
        this->waitMs = 1000.0 / fps;
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::SetQuality(int pQuality) {
        this->quality = pQuality;
    }

    template <typename TFrame, bool SSL>
    int LiveVideo<TFrame, SSL>::GetTotalClients() {
        std::lock_guard<std::mutex> g_clients(mtxClients);

        return clients.size();
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::PostStop() {}

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::PreStart() {}

    template <typename TFrame, bool SSL>
    LiveViewStatus LiveVideo<TFrame, SSL>::GetStatus() {
        return status;
    }
}  // namespace Web