#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "Streaming/IBroadcastService.hpp"
#include "Streaming/Video/LiveViewStatus.hpp"
#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Camera/IFrameSubscriber.hpp"
#include "observer/Semaphore.hpp"
#include "observer/Timer.hpp"
#include "observer/Utils/SpecialEnums.hpp"

constexpr double SECONDS_TO_ZOMBIE = 1 * 30;

namespace Web::Streaming::Video {
    /**
     * @brief This class is responsible for sending the frames to the clients.
     * Note that it doesn't have any logic to get the frames. It is the
     * responsibility of the derived class to implement it.
     * For example you could have a class that gets the frames from a camera,
     * and another class that generates random frames.
     *
     * @tparam SSL
     * @tparam Client The type of the client.
     */
    template <bool SSL, typename Client>
    class StreamWriter : public Observer::Functionality {
       public:
        /**
         * @brief Construct a new Stream Writer object
         *
         * @param quality jpg compression quality
         * @param service service to send the frames to
         * @param maxFps If set, it will limit the fps to this value (in the
         * case the source is faster than this value). If not set, it will send
         * the frames whenever they are received.
         */
        StreamWriter(std::optional<int> maxFps, int quality,
                     IBroadcastService<SSL, Client>* service);

        LiveViewStatus GetStatus();

        virtual ~StreamWriter() = default;

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
        Semaphore smpFrame;
        Observer::Frame frame;
        std::mutex mtxFrame;
        LiveViewStatus status;
        Observer::Timer<std::chrono::seconds> timerZombie;
        IBroadcastService<SSL, Client>* service;

       private:
        double waitMs;
        int quality;
        int id;
    };

    template <bool SSL, typename Client>
    StreamWriter<SSL, Client>::StreamWriter(
        std::optional<int> maxFps, int pQuality,
        IBroadcastService<SSL, Client>* pService)
        : quality(pQuality),
          frame(Observer::Size(640, 360), 3),
          service(pService) {
        if (maxFps.has_value()) {
            this->SetFPS(maxFps.value());
        } else {
            this->waitMs = 0;
        }

        Observer::set_flag(status, LiveViewStatus::CLOSED);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::NewValidFrameReceived() {
        this->smpFrame.release();
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::InternalStart() {
        std::vector<unsigned char> buffer;
        this->PreStart();

        if (Observer::has_flag(status, LiveViewStatus::STOPPED)) {
            Observer::clear_flag(status, LiveViewStatus::STOPPED);
        }

        Observer::set_flag(status, LiveViewStatus::RUNNING);

        while (this->running) {
            this->GetNextFrame();

            if (this->service->GetTotalClients() > 0) {
                this->timerZombie.Restart();
            }

            if (this->IsZombie()) {
                OBSERVER_TRACE(
                    "Live view was stopped because it had no clients after {} "
                    "seconds",
                    SECONDS_TO_ZOMBIE);

                this->PostStop();
                return;
            }

            if (smpFrame.acquire_timeout<500>()) {
                this->mtxFrame.lock();

                if (!this->frame.IsEmpty()) {
                    this->frame.EncodeImage(".jpg", this->quality, buffer);

                    this->mtxFrame.unlock();

                    this->service->SendToClients((char*)buffer.data(),
                                                 buffer.size());
                } else {
                    this->mtxFrame.unlock();
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds((int)waitMs));
            }
        }

        Observer::clear_flag(status, LiveViewStatus::RUNNING);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::SetFPS(double fps) {
        OBSERVER_ASSERT(fps > 0, "fps must be greater than 0");
        this->waitMs = 1000.0 / fps;
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::SetQuality(int pQuality) {
        OBSERVER_ASSERT(pQuality >= 0 && pQuality <= 100,
                        "quality must be between 0 and 100");
        this->quality = pQuality;
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::PostStop() {
        Observer::clear_flag(this->status, LiveViewStatus::RUNNING);
        this->status |= LiveViewStatus::STOPPED;
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::PreStart() {
        Observer::clear_flag(this->status, LiveViewStatus::STOPPED);
    }

    template <bool SSL, typename Client>
    LiveViewStatus StreamWriter<SSL, Client>::GetStatus() {
        return status;
    }

    template <bool SSL, typename Client>
    bool StreamWriter<SSL, Client>::IsZombie() {
        return this->timerZombie.GetDuration() >= SECONDS_TO_ZOMBIE;
    }
}  // namespace Web::Streaming::Video