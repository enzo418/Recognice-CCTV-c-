#pragma once

#include <cstdint>

#include "observer/Log/log.hpp"
#include "webrtc/broadrtc/Observers.hpp"

namespace Web::Streaming::WebRTC {
    class TerminatorWithLoggingConnectionObserver
        : public broadrtc::BasicConnectionObserver {
       public:
        void OnConnectionChange(
            webrtc::PeerConnectionInterface::PeerConnectionState new_state)
            override;
    };
}  // namespace Web::Streaming::WebRTC