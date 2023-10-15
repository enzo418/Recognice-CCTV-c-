#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string_view>

#include "LiveViewExceptions.hpp"
#include "SocketData.hpp"
#include "Streaming/Video/StreamWriter.hpp"
#include "observer/Domain/BufferedSource.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Timer.hpp"
#include "observer/Utils/SpecialEnums.hpp"

namespace Web::Streaming::Video {
    template <bool SSL, typename Client>
    class CameraLiveVideo final : public StreamWriter<SSL, Client> {
       public:
        CameraLiveVideo(const std::string& pCameraUri, int quality,
                        IBroadcastService<SSL, Client>* service);
        virtual ~CameraLiveVideo() {}

       public:
        std::string_view GetURI();

       protected:
        void GetNextFrame() override;

        void PostStop() override;
        void PreStart() override;

       private:
        /**
         * @brief Open the camera. Also sets and clear the flags open/closed, or
         * error in case it couldn't open a connection with the camera.
         *
         * @return true could open
         * @return false
         */
        void OpenCamera();

       private:
        Observer::BufferedSource source;
        std::string cameraUri;

       private:
        typedef LiveViewStatus Status;
    };

    template <bool SSL, typename Client>
    CameraLiveVideo<SSL, Client>::CameraLiveVideo(
        const std::string& pCameraUri, int pQuality,
        IBroadcastService<SSL, Client>* service)
        : StreamWriter<SSL, Client>(std::nullopt, pQuality, service),
          cameraUri(pCameraUri) {
        this->OpenCamera();

        if (Observer::has_flag(this->status, Status::OPEN)) {
            StreamWriter<SSL, Client>::SetFPS(source.GetFPS());
        }
    }

    template <bool SSL, typename Client>
    void CameraLiveVideo<SSL, Client>::GetNextFrame() {
        if (source.IsFrameAvailable()) {
            /**
             * There is no need to lock the frame mutex since it's the only
             * way to get the frames and its called from the main loop.
             */
            this->frame = source.GetFrame();

            this->NewValidFrameReceived();
        }
    }

    template <bool SSL, typename Client>
    std::string_view CameraLiveVideo<SSL, Client>::GetURI() {
        return this->cameraUri;
    }

    template <bool SSL, typename Client>
    void CameraLiveVideo<SSL, Client>::PostStop() {
        StreamWriter<SSL, Client>::PostStop();
        source.Close();
        Observer::clear_flag(this->status, Status::OPEN);
        this->status |= Status::CLOSED;
    }

    template <bool SSL, typename Client>
    void CameraLiveVideo<SSL, Client>::PreStart() {
        StreamWriter<SSL, Client>::PreStart();

        if (!Observer::has_flag(this->status, Status::OPEN) ||
            Observer::has_flag(this->status, Status::STOPPED)) {
            OBSERVER_TRACE("Opening camera source: {}", cameraUri);
            this->OpenCamera();
        } else {
            OBSERVER_TRACE("Source was running: {}", cameraUri);
        }

        this->source.Start();
    }

    template <bool SSL, typename Client>
    void CameraLiveVideo<SSL, Client>::OpenCamera() {
        if (source.TryOpen(cameraUri)) {
            Observer::clear_flag(this->status, Status::CLOSED);
            Observer::clear_flag(this->status, Status::ERROR);

            Observer::set_flag(this->status, Status::OPEN);
        } else {
            Observer::set_flag(this->status, Status::ERROR);
        }
    }

}  // namespace Web::Streaming::Video