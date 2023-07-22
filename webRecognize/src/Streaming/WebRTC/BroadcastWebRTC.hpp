#pragma once

#include <memory>

#include "GenericBroadcastSource.hpp"
#include "Server/RecognizeContext.hpp"
#include "Streaming/WebRTC/ObserverBroadcastSource.hpp"
#include "broadrtc/BroadcastSource.hpp"
#include "observer/Log/log.hpp"
#include "webrtc/broadrtc/BroadcastWebRTC.hpp"

namespace Web::Streaming::WebRTC {
    class MultiBroadcastWebRTC final : public broadrtc::BroadcastWebRTC {
       public:
        constexpr static const char* OBSERVER_SRC_TAG = "observer";

       public:
        MultiBroadcastWebRTC(RecognizeContext* pRecognizeCtx)
            : recognizeCtx(pRecognizeCtx) {}

        void OnChannelNotFound(const std::string& sourceId) override;
        void OnObserverStopped();

       private:
        RecognizeContext* recognizeCtx;
    };
}  // namespace Web::Streaming::WebRTC