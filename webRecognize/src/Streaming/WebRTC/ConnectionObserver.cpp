#include "ConnectionObserver.hpp"
namespace Web::Streaming::WebRTC {
    void TerminatorWithLoggingConnectionObserver::OnConnectionChange(
        webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
        switch (new_state) {
            case webrtc::PeerConnectionInterface::PeerConnectionState::kFailed:
                // The previous state doesn't apply (it's not closed) and any
                // RTCIceTransports are in the "failed" state.
                OBSERVER_ASSERT(m_peerConnection.get() != nullptr,
                                "Peer connection is null");

                OBSERVER_INFO("Terminating connection to {}",
                              (intptr_t)m_peerConnection.get());

                m_peerConnection->Close();
                break;
            case webrtc::PeerConnectionInterface::PeerConnectionState::kClosed:
                OBSERVER_TRACE("Connection to {} closed successfully",
                               (intptr_t)m_peerConnection.get());
                break;
            case webrtc::PeerConnectionInterface::PeerConnectionState::
                kConnected:
                OBSERVER_TRACE("Connection to {} established successfully",
                               (intptr_t)m_peerConnection.get());
                break;
            default:
                break;
        }
    }
}  // namespace Web::Streaming::WebRTC