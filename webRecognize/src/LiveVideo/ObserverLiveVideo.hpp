#pragma once

#include "LiveVideo.hpp"

namespace Web {
    template <typename TFrame, bool SSL>
    class ObserverLiveVideo : public LiveVideo<TFrame, SSL>,
                              public ISubscriber<TFrame> {
       public:
        typedef uWS::WebSocket<SSL, true, PerSocketData> WebSocketClient;

       public:
        ObserverLiveVideo(int fps, int quality);

        virtual ~ObserverLiveVideo() {};

        void update(TFrame frame) override;

       private:
        void GetNextFrame() override;
    };

    template <typename TFrame, bool SSL>
    ObserverLiveVideo<TFrame, SSL>::ObserverLiveVideo(int pFps, int pQuality)
        : LiveVideo<TFrame, SSL>(pFps, pQuality) {}

    template <typename TFrame, bool SSL>
    void ObserverLiveVideo<TFrame, SSL>::update(TFrame pFrame) {
        std::lock_guard<std::mutex> guard_f(this->mtxFrame);
        // OBSERVER_TRACE("Updating image");

        Observer::Size size =
            Observer::ImageTransformation<TFrame>::GetSize(pFrame);

        if (!size.empty()) {
            this->frame = pFrame;
            // Observer::ImageTransformation<TFrame>::CopyImage(frame,
            //                                                  this->frame);

            this->NewValidFrameReceived();
        }
    }

    template <typename TFrame, bool SSL>
    void ObserverLiveVideo<TFrame, SSL>::GetNextFrame() {
        // Do nothing, all frames are received from update calls
    }
}  // namespace Web