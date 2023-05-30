#pragma once

#include "SocketData.hpp"
#include "Streaming/Video/StreamWriter.hpp"
#include "uWebSockets/WebSocket.h"

namespace Web::Streaming::Video::Ws {
    template <bool SSL, typename Client>
    class ObserverLiveVideo final : public StreamWriter<SSL, Client>,
                                    public ISubscriber<Observer::Frame> {
       public:
        ObserverLiveVideo(int fps, int quality,
                          IStreamingService<SSL, Client>* service);

        virtual ~ObserverLiveVideo() {};

        void update(Observer::Frame frame) override;

       private:
        void GetNextFrame() override;
    };

    template <bool SSL, typename Client>
    ObserverLiveVideo<SSL, Client>::ObserverLiveVideo(
        int pFps, int pQuality, IStreamingService<SSL, Client>* service)
        : StreamWriter<SSL, Client>(pFps, pQuality, service) {}

    template <bool SSL, typename Client>
    void ObserverLiveVideo<SSL, Client>::update(Observer::Frame pFrame) {
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

    template <bool SSL, typename Client>
    void ObserverLiveVideo<SSL, Client>::GetNextFrame() {
        // Do nothing, all frames are received from update calls
    }
}  // namespace Web::Streaming::Video::Ws