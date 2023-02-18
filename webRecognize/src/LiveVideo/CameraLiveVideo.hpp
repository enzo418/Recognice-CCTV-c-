#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string_view>

#include "LiveVideo.hpp"
#include "LiveViewExceptions.hpp"
#include "observer/Domain/BufferedSource.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Timer.hpp"

namespace Web {
    template <bool SSL>
    class CameraLiveVideo final : public LiveVideo<SSL> {
       public:
        CameraLiveVideo(const std::string& pCameraUri, int quality);
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

    template <bool SSL>
    CameraLiveVideo<SSL>::CameraLiveVideo(const std::string& pCameraUri,
                                          int pQuality)
        : LiveVideo<SSL>(100, pQuality), cameraUri(pCameraUri) {
        this->OpenCamera();

        if (Observer::has_flag(this->status, Status::OPEN)) {
            LiveVideo<SSL>::SetFPS(source.GetFPS());
        }
    }

    template <bool SSL>
    void CameraLiveVideo<SSL>::GetNextFrame() {
        if (source.IsFrameAvailable()) {
            /**
             * There is no need to lock the frame mutex since it's the only
             * way to get the frames and its called from the main loop.
             */
            this->frame = source.GetFrame();

            this->NewValidFrameReceived();
        }
    }

    template <bool SSL>
    std::string_view CameraLiveVideo<SSL>::GetURI() {
        return this->cameraUri;
    }

    template <bool SSL>
    void CameraLiveVideo<SSL>::PostStop() {
        source.Close();
    }

    template <bool SSL>
    void CameraLiveVideo<SSL>::PreStart() {
        if (!Observer::has_flag(this->status, Status::OPEN) ||
            Observer::has_flag(this->status, Status::STOPPED)) {
            OBSERVER_TRACE("Opening camera source: {}", cameraUri);
            this->OpenCamera();
        } else {
            OBSERVER_TRACE("Source was running: {}", cameraUri);
        }

        this->source.Start();
    }

    template <bool SSL>
    void CameraLiveVideo<SSL>::OpenCamera() {
        if (source.TryOpen(cameraUri)) {
            Observer::clear_flag(this->status, Status::CLOSED);
            Observer::clear_flag(this->status, Status::ERROR);

            Observer::set_flag(this->status, Status::OPEN);
        } else {
            Observer::set_flag(this->status, Status::ERROR);
        }
    }

}  // namespace Web