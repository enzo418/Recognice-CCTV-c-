#pragma once

#include "LiveVideo.hpp"

namespace Web {
    template <bool SSL>
    class ObserverLiveVideo : public LiveVideo<SSL>,
                              public ISubscriber<Observer::Frame> {
       public:
        typedef uWS::WebSocket<SSL, true, PerSocketData> WebSocketClient;

       public:
        ObserverLiveVideo(int fps, int quality);

        virtual ~ObserverLiveVideo() {};

        void update(Observer::Frame frame) override;

       private:
        void GetNextFrame() override;
    };

    template <bool SSL>
    ObserverLiveVideo<SSL>::ObserverLiveVideo(int pFps, int pQuality)
        : LiveVideo<SSL>(pFps, pQuality) {}

    template <bool SSL>
    void ObserverLiveVideo<SSL>::update(Observer::Frame pFrame) {
        std::lock_guard<std::mutex> guard_f(this->mtxFrame);
        // OBSERVER_TRACE("Updating image");

        Observer::Size size = pFrame.GetSize();

        if (!size.empty()) {
            this->frame = pFrame;
            // Observer::ImageTransformation<TFrame>::CopyImage(frame,
            //                                                  this->frame);

            this->NewValidFrameReceived();
        }
    }

    template <bool SSL>
    void ObserverLiveVideo<SSL>::GetNextFrame() {
        // Do nothing, all frames are received from update calls
    }
}  // namespace Web