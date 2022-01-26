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
#include "../SocketData.hpp"
#include "../WebsocketService.hpp"

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
    class LiveVideo : public IFunctionality,
                      public WebsocketService<SSL, PerSocketData> {
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

        LiveViewStatus GetStatus();

       private:
        void InternalStart();

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
        std::thread thread;
    };

    template <typename TFrame, bool SSL>
    LiveVideo<TFrame, SSL>::LiveVideo(int pFps, int pQuality)
        : waitMs(1000.0 / (double)pFps), quality(pQuality) {
        Observer::set_flag(status, LiveViewStatus::CLOSED);
        Observer::set_flag(status, LiveViewStatus::STOPPED);

        frame = Observer::ImageTransformation<TFrame>::BlackImage();
    }

    template <typename TFrame, bool SSL>
    LiveVideo<TFrame, SSL>::~LiveVideo() {
        this->Stop();
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::Start() {
        OBSERVER_ASSERT(!running, "Live view alredy running!");

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
    void LiveVideo<TFrame, SSL>::SetFPS(double fps) {
        this->waitMs = 1000.0 / fps;
    }

    template <typename TFrame, bool SSL>
    void LiveVideo<TFrame, SSL>::SetQuality(int pQuality) {
        this->quality = pQuality;
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