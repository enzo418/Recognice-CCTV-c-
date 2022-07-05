#pragma once

#include <mutex>
#include <string_view>

#include "LiveVideo.hpp"
#include "LiveViewExceptions.hpp"
#include "observer/Implementation.hpp"

namespace Web {
    template <bool SSL>
    class CameraLiveVideo : public LiveVideo<SSL> {
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
         * error in case it couldn't open a connectoin with the camera.
         *
         * @return true could open
         * @return false
         */
        void OpenCamera();

       private:
        Observer::VideoSource source;
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
        /**
         * There is no need to lock the frame mutex since it's the only
         * way to get the frames and its called from the main loop.
         */
        source.GetNextFrame(this->frame);

        this->NewValidFrameReceived();
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
        this->OpenCamera();
    }

    template <bool SSL>
    void CameraLiveVideo<SSL>::OpenCamera() {
        source.Open(cameraUri);

        if (source.isOpened()) {
            Observer::clear_flag(this->status, Status::CLOSED);
            Observer::clear_flag(this->status, Status::ERROR);

            Observer::set_flag(this->status, Status::OPEN);
        } else {
            Observer::set_flag(this->status, Status::ERROR);
        }
    }
}  // namespace Web