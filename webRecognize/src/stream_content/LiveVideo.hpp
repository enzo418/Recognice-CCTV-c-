#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../../../recognize/Observer/src/IFunctionality.hpp"
#include "../../../recognize/Observer/src/ImageTransformation.hpp"
#include "../../../recognize/Observer/src/Log/log.hpp"
#include "../../../recognize/Observer/src/Pattern/Camera/IFrameSubscriber.hpp"
#include "../../uWebSockets/src/App.h"

namespace Web {
    template <typename TFrame, bool SSL>
    class LiveVideo : public IFunctionality,
                      public Observer::IFrameSubscriber<TFrame> {
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

        void AddClient(uWS::HttpResponse<SSL>* res);

        void update(int camerapos, TFrame frame) override;

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
        std::vector<uWS::HttpResponse<SSL>*> clients;
        std::mutex mtxClients;

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
    void LiveVideo<TFrame, SSL>::AddClient(uWS::HttpResponse<SSL>* res) {
        const std::string status = "HTTP/1.0 206 Partial Content\r\n";

        const std::string header =
            "Accept-Range: bytes\r\n"
            "Connection: close\r\n"
            "Max-Age: 0\r\n"
            "Expires: 0\r\n"
            "Cache-Control: no-cache, private\r\n"
            "Pragma: no-cache\r\n"
            "Content-Type: multipart/x-mixed-replace; "
            "boundary=frame\r\n"
            "\r\n";

        // this commented code should be the way this should be done as far as
        // the library tell us, but it  doens't seems to be working for some
        // reason. Even with tryEnd, end, or write(null). For that reason the
        // solution i found was to write the header and status directly on the
        // stream.

        /*
        res->writeStatus("206 Partial Content")
        ->writeHeader("Connection", "close")
        ->writeHeader("Accept-Ranges", "bytes")
        ->writeHeader("Max-Age", "0")
        ->writeHeader("Expires", "0")
        ->writeHeader("Pragma", "no-cache")
        ->writeHeader("Cache-Control", "no-cache, private")
        ->writeHeader("Content-Type",
                        "multipart/x-mixed-replace; "
                        "boundary=frame");*/

        res->writeRaw(std::string_view(status.c_str(), status.size()));
        res->writeRaw(std::string_view(header.c_str(), header.size()));

        std::lock_guard<std::mutex> guard_c(this->mtxClients);
        this->clients.push_back(res);
        OBSERVER_TRACE("New Client!");
        // }
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::update(int camerapos, TFrame frame) {
        this->UpdateFrame(frame);
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::UpdateFrame(TFrame& pFrame) {
        std::lock_guard<std::mutex> guard_f(this->mtxFrame);
        OBSERVER_TRACE("Updating image");

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
            if (!this->clients.empty()) {
                this->mtxFrame.lock();
                if (!this->encoded && this->imageReady) {
                    OBSERVER_TRACE("Encoding image");
                    Observer::ImageTransformation<TFrame>::EncodeImage(
                        ".jpg", this->frame, this->quality, buffer);
                    this->encoded = true;
                }
                this->mtxFrame.unlock();

                this->SendToClients((char*)buffer.data(), buffer.size());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds((int)waitMs));
        }
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::SendToClients(char* data, int size) {
        // TODO: This lock is not even necessary since we know the size since
        // then
        std::lock_guard<std::mutex> guard_c(this->mtxClients);

        static const std::string boundary =
            "--frame\r\nContent-Type: image/jpeg\r\n\r\n";

        std::string buf = boundary + std::string(data, size);

        int clientsCount = this->clients.size();
        for (int i = 0; i < clientsCount; i++) {
            const bool sucessBoundary = this->clients[i]->writeRaw(
                std::string_view(boundary.c_str(), boundary.size()));

            if (!sucessBoundary ||
                !this->clients[i]->writeRaw(std::string_view(data, size))) {
                OBSERVER_INFO("Client deleted");
                // client disconnected
                this->clients.erase(this->clients.begin() + i);
                i--;
                clientsCount--;
            } else {
                OBSERVER_TRACE("Sended image to client, total: {}",
                               clientsCount);
            }
        }
    }
}  // namespace Web