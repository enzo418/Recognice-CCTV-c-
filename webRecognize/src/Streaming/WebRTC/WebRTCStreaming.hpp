#pragma once

#include "BroadcastWebRTC.hpp"
#include "ConnectionObserver.hpp"
#include "Server/RecognizeContext.hpp"
#include "nlohmann/json.hpp"
#include "observer/Log/log.hpp"
#include "uWebSockets/HttpResponse.h"
#include "webrtc/webrtc_config.h"

namespace Web::Streaming::WebRTC {

    template <bool SSL>
    class WebRTCStreamingService final {
       public:
        typedef typename uWS::HttpResponse<SSL> Client;

        WebRTCStreamingService(RecognizeContext* pRecognizeCtx)
            : recognizeCtx(pRecognizeCtx), broadcast(pRecognizeCtx) {
            if (!wasInitialized) {
                broadrtc::BroadcastWebRTC::InitWebRTC(
                    broadrtc::BroadcastWebRTC::LS_WARNING);
                wasInitialized = true;
            }
        }

        ~WebRTCStreamingService() { broadrtc::BroadcastWebRTC::DeinitWebRTC(); }

        /**
         * @brief Stream a source.
         * Will write a json object with:
         * - clientId: the id of the client, used to send the answer.
         * - offer: the json SDP offer
         *
         * @param uri
         * @param client
         * @return true success
         */
        bool Stream(const std::string& uri, Client* client);

        /**
         * @brief Same as above but with uri "observer".
         *
         * @param client
         * @return true
         * @return false
         */
        bool StreamObserver(Client* client);

        /**
         * @brief Set the peer answer.
         *
         * @param clientId
         * @param answer the json SDP answer
         */
        void SetPeerAnswer(const std::string& clientId,
                           const std::string& answer);

        /**
         * @brief End the connection with the peer.
         *
         * @param clientId
         */
        void PeerHangup(const std::string& clientId);

        /**
         * @brief Called when observer was stopped.
         * This will only stop emitting frames from the observer source.
         */
        void OnObserverStopped();

       private:
        MultiBroadcastWebRTC broadcast;

        static bool wasInitialized;

        RecognizeContext* recognizeCtx;
    };

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::wasInitialized = false;

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::Stream(const std::string& uri,
                                             Client* client) {
        OBSERVER_INFO("Stream {}", uri);

        try {
            auto [clientId, offer] = broadcast.ConnectToChannel(
                uri,
                std::make_unique<TerminatorWithLoggingConnectionObserver>());

            client->endJson((nlohmann::json {
                                 {"clientId", clientId},
                                 {"offer", offer},
                             })
                                .dump());
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Failed to peer to uri {}: {}", uri, e.what());
            return false;
        }

        return true;
    }

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::StreamObserver(Client* client) {
        return Stream(MultiBroadcastWebRTC::OBSERVER_SRC_TAG, client);
    }

    template <bool SSL>
    void WebRTCStreamingService<SSL>::SetPeerAnswer(const std::string& clientId,
                                                    const std::string& answer) {
        OBSERVER_INFO("SetPeerAnswer {}", clientId);

        try {
            broadcast.SetAnswer(clientId, answer);
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Failed to set peer answer {}: {}", clientId,
                           e.what());
        }
    }

    template <bool SSL>
    void WebRTCStreamingService<SSL>::PeerHangup(const std::string& clientId) {
        OBSERVER_INFO("PeerHangup {}", clientId);

        try {
            broadcast.EndConnection(clientId);
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Failed to hangup peer {}: {}", clientId, e.what());
        }
    }

    template <bool SSL>
    void WebRTCStreamingService<SSL>::OnObserverStopped() {
        broadcast.OnObserverStopped();
    }

}  // namespace Web::Streaming::WebRTC