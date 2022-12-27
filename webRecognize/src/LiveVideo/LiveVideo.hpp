#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../../../recognize/Observer/vendor/bitmask_operators.hpp"
#include "../SocketData.hpp"
#include "../WebsocketService.hpp"
#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Camera/IFrameSubscriber.hpp"
#include "observer/Timer.hpp"
#include "observer/Utils/SpecialEnums.hpp"

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

constexpr double SECONDS_TO_ZOMBIE = 1 * 30;

namespace Web {
    template <bool SSL>
    class LiveVideo : public Observer::Functionality,
                      public WebsocketService<SSL, PerSocketData> {
       public:
        LiveVideo(int fps, int quality);

        LiveViewStatus GetStatus();

        virtual ~LiveVideo() = default;

        /**
         * @brief If it went live and nobody is using its frame
         *
         * @return true
         * @return false
         */
        bool IsZombie();

       private:
        void InternalStart();

       protected:
        void NewValidFrameReceived();

        /**
         * @brief Updates this->frames
         */
        virtual void GetNextFrame() = 0;

        /**
         * @brief Called after Stop is called.
         *
         */
        virtual void PostStop();

        /**
         * @brief Called once Start is called.
         *
         */
        virtual void PreStart();

        void SetFPS(double fps);

        void SetQuality(int quality);

       protected:
        Observer::Frame frame;
        std::mutex mtxFrame;
        LiveViewStatus status;
        Observer::Timer<std::chrono::seconds> timerZombie;

       private:
        bool encoded {false};
        bool imageReady {false};

       private:
        double waitMs;
        int quality;
        int id;
    };

    template <bool SSL>
    LiveVideo<SSL>::LiveVideo(int pFps, int pQuality)
        : waitMs(1000.0 / (double)pFps),
          quality(pQuality),
          frame(Observer::Size(640, 360), 3) {
        Observer::set_flag(status, LiveViewStatus::CLOSED);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <bool SSL>
    void LiveVideo<SSL>::NewValidFrameReceived() {
        this->imageReady = true;
        this->encoded = false;
    }

    template <bool SSL>
    void LiveVideo<SSL>::InternalStart() {
        std::vector<unsigned char> buffer;
        this->PreStart();

        if (Observer::has_flag(status, LiveViewStatus::STOPPED)) {
            Observer::clear_flag(status, LiveViewStatus::STOPPED);
        }

        Observer::set_flag(status, LiveViewStatus::RUNNING);

        while (this->running) {
            this->GetNextFrame();

            this->mtxFrame.lock();

            if (this->GetTotalClients() > 0) this->timerZombie.Restart();

            if (this->IsZombie()) {
                OBSERVER_TRACE(
                    "Live view was stopped because it had no clients after {} "
                    "seconds",
                    SECONDS_TO_ZOMBIE);

                this->Stop();
            }

            if (!this->encoded && this->imageReady && !this->frame.IsEmpty()) {
                this->frame.EncodeImage(".jpg", this->quality, buffer);
                this->encoded = true;
            }
            this->mtxFrame.unlock();

            this->SendToClients((char*)buffer.data(), buffer.size());

            std::this_thread::sleep_for(std::chrono::milliseconds((int)waitMs));
        }

        Observer::clear_flag(status, LiveViewStatus::RUNNING);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <bool SSL>
    void LiveVideo<SSL>::SetFPS(double fps) {
        this->waitMs = 1000.0 / fps;
    }

    template <bool SSL>
    void LiveVideo<SSL>::SetQuality(int pQuality) {
        this->quality = pQuality;
    }

    template <bool SSL>
    void LiveVideo<SSL>::PostStop() {}

    template <bool SSL>
    void LiveVideo<SSL>::PreStart() {}

    template <bool SSL>
    LiveViewStatus LiveVideo<SSL>::GetStatus() {
        return status;
    }

    template <bool SSL>
    bool LiveVideo<SSL>::IsZombie() {
        return this->timerZombie.GetDuration() >= SECONDS_TO_ZOMBIE;
    }
}  // namespace Web