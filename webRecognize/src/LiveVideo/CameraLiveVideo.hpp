#pragma once

#include <mutex>
#include <string_view>

#include "../../../recognize/Observer/src/Implementation.hpp"
#include "LiveVideo.hpp"
#include "LiveViewExceptions.hpp"

namespace Web {
    template <typename TFrame, bool SSL>
    class CameraLiveVideo : public LiveVideo<TFrame, SSL> {
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

    template <typename TFrame, bool SSL>
    CameraLiveVideo<TFrame, SSL>::CameraLiveVideo(const std::string& pCameraUri,
                                                  int pQuality)
        : LiveVideo<TFrame, SSL>(100, pQuality), cameraUri(pCameraUri) {
        this->OpenCamera();

        if (Observer::has_flag(this->status, Status::OPEN)) {
            LiveVideo<TFrame, SSL>::SetFPS(source.GetFPS());
        }
    }

    template <typename TFrame, bool SSL>
    void CameraLiveVideo<TFrame, SSL>::GetNextFrame() {
        /**
         * There is no need to lock the frame mutex since it's the only
         * way to get the frames and its called from the main loop.
         */
        source.GetNextFrame(this->frame);

        this->NewValidFrameReceived();
    }

    template <typename TFrame, bool SSL>
    std::string_view CameraLiveVideo<TFrame, SSL>::GetURI() {
        return this->cameraUri;
    }

    template <typename TFrame, bool SSL>
    void CameraLiveVideo<TFrame, SSL>::PostStop() {
        source.Close();
    }

    template <typename TFrame, bool SSL>
    void CameraLiveVideo<TFrame, SSL>::PreStart() {
        this->OpenCamera();
    }

    template <typename TFrame, bool SSL>
    void CameraLiveVideo<TFrame, SSL>::OpenCamera() {
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