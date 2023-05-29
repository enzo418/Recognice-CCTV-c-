#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "Streaming/IStreamingService.hpp"
#include "Streaming/Video/LiveViewStatus.hpp"
#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Camera/IFrameSubscriber.hpp"
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
        StreamWriter(int fps, int quality,
                     IStreamingService<SSL, Client>* service);

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
        Observer::Frame frame;
        std::mutex mtxFrame;
        LiveViewStatus status;
        Observer::Timer<std::chrono::seconds> timerZombie;
        IStreamingService<SSL, Client>* service;

       private:
        bool encoded {false};
        bool imageReady {false};

       private:
        double waitMs;
        int quality;
        int id;
    };

    template <bool SSL, typename Client>
    StreamWriter<SSL, Client>::StreamWriter(
        int pFps, int pQuality, IStreamingService<SSL, Client>* pService)
        : waitMs(1000.0 / (double)pFps),
          quality(pQuality),
          frame(Observer::Size(640, 360), 3),
          service(pService) {
        Observer::set_flag(status, LiveViewStatus::CLOSED);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::NewValidFrameReceived() {
        this->imageReady = true;
        this->encoded = false;
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

            this->mtxFrame.lock();

            if (this->service->GetTotalClients() > 0)
                this->timerZombie.Restart();

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

            this->service->SendToClients((char*)buffer.data(), buffer.size());

            std::this_thread::sleep_for(std::chrono::milliseconds((int)waitMs));
        }

        Observer::clear_flag(status, LiveViewStatus::RUNNING);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::SetFPS(double fps) {
        this->waitMs = 1000.0 / fps;
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::SetQuality(int pQuality) {
        this->quality = pQuality;
    }

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::PostStop() {}

    template <bool SSL, typename Client>
    void StreamWriter<SSL, Client>::PreStart() {}

    template <bool SSL, typename Client>
    LiveViewStatus StreamWriter<SSL, Client>::GetStatus() {
        return status;
    }

    template <bool SSL, typename Client>
    bool StreamWriter<SSL, Client>::IsZombie() {
        return this->timerZombie.GetDuration() >= SECONDS_TO_ZOMBIE;
    }
}  // namespace Web::Streaming::Video