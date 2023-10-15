#pragma once

#include "BroadcastWebRTC.hpp"
#include "ConnectionObserver.hpp"
#include "Controller/ControllerUtils.hpp"
#include "Server/RecognizeContext.hpp"
#include "Streaming/IStreamingSetupService.hpp"
#include "nlohmann/json.hpp"
#include "observer/Log/log.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/HttpResponse.h"
#include "webrtc/webrtc_config.h"

namespace Web::Streaming::WebRTC {

    template <bool SSL>
    class WebRTCStreamingService final : public IStreamingSetupService {
       public:
        typedef typename uWS::HttpResponse<SSL> Client;

        WebRTCStreamingService(uWS::App* app, RecognizeContext* pRecognizeCtx)
            : recognizeCtx(pRecognizeCtx), broadcast(pRecognizeCtx) {
            if (!wasInitialized) {
                broadrtc::BroadcastWebRTC::InitWebRTC(
                    broadrtc::BroadcastWebRTC::LS_WARNING);
                wasInitialized = true;
            }

            // It should be POST /api/stream/:client_id/answer but uWebSockets
            // has a hard time parsing the numeric parameters...
            app->post("/api/stream/webrtc/answer",
                      [this](auto* res, auto* req) {
                          READ_JSON_BODY(res, req, SetPeerAnswer);
                      });

            // I would like to use DELETE method, /api/stream/:client_id
            app->post("/api/stream/webrtc/hangup",
                      [this](auto* res, auto* req) {
                          READ_JSON_BODY(res, req, PeerHangup);
                      });
        }

        ~WebRTCStreamingService() { broadrtc::BroadcastWebRTC::DeinitWebRTC(); }

        bool PrepareStream(
            const std::string& uri,
            std::unordered_map<std::string, std::string>& result) override;

        bool PrepareStreamObserver(
            std::unordered_map<std::string, std::string>& result) override;

        void OnObserverStopped() override;

        /**
         * @brief Set the peer answer.
         *
         * @param clientId
         * @param answer the json SDP answer
         * @return true success
         */
        bool SetPeerAnswer(const std::string& clientId,
                           const std::string& answer);

        /**
         * @brief End the connection with the peer.
         *
         * @param clientId
         */
        void PeerHangup(const std::string& clientId);

       private:
        /**
         * @brief Set the peer answer.
         * Receives a json object with:
         * - clientId: the id of the client, used to send the answer.
         * - answer: the json SDP answer
         *
         * @param res
         * @param req
         */
        void SetPeerAnswer(auto* res, auto* req, const nlohmann::json& body);

        /**
         * @brief End the connection with the peer.
         * Receives a json object with:
         * - clientId: the id of the client, used to send the answer.
         *
         * @param res
         * @param req
         */
        void PeerHangup(auto* res, auto* req, const nlohmann::json& body);

       private:
        MultiBroadcastWebRTC broadcast;

        static bool wasInitialized;

        RecognizeContext* recognizeCtx;
    };

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::wasInitialized = false;

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::PrepareStream(
        const std::string& uri,
        std::unordered_map<std::string, std::string>& result) {
        OBSERVER_INFO("Stream {}", uri);

        try {
            auto [clientId, offer] = broadcast.ConnectToChannel(
                uri,
                std::make_unique<TerminatorWithLoggingConnectionObserver>());

            result["clientId"] = clientId;
            result["offer"] = offer;
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Failed to peer to uri {}: {}", uri, e.what());
            return false;
        }

        return true;
    }

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::PrepareStreamObserver(
        std::unordered_map<std::string, std::string>& result) {
        return PrepareStream(MultiBroadcastWebRTC::OBSERVER_SRC_TAG, result);
    }

    template <bool SSL>
    bool WebRTCStreamingService<SSL>::SetPeerAnswer(const std::string& clientId,
                                                    const std::string& answer) {
        OBSERVER_INFO("SetPeerAnswer {}", clientId);

        try {
            broadcast.SetAnswer(clientId, answer);
            return true;
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Failed to set peer answer {}: {}", clientId,
                           e.what());
            return false;
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

    template <bool SSL>
    void WebRTCStreamingService<SSL>::SetPeerAnswer(
        auto* res, auto* req, const nlohmann::json& body) {
        if (!body.contains("clientId") || !body.contains("answer")) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson((nlohmann::json {{"title",
                                                   "clientId and answer are "
                                                   "required"}})
                                     .dump());
            return;
        }

        std::string clientId;

        if (body["clientId"].is_string()) {
            clientId = body["clientId"];
        } else if (body["clientId"].is_number()) {
            clientId = std::to_string(body["clientId"].get<int>());
        } else {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson((nlohmann::json {{"title",
                                                   "clientId must be a string "
                                                   "or a number"}})
                                     .dump());
            return;
        }

        std::string answer;

        if (body["answer"].is_string()) {
            answer = body["answer"];
        } else {
            answer = body["answer"].dump();
        }

        if (this->SetPeerAnswer(clientId, answer)) {
            res->writeStatus(HTTP_200_OK)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"success\": true}");
        } else {
            res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)
                ->endProblemJson(
                    (nlohmann::json {{"title", "Failed to set peer answer"}})
                        .dump());
        }
    }

    template <bool SSL>
    void WebRTCStreamingService<SSL>::PeerHangup(auto* res, auto* req,
                                                 const nlohmann::json& body) {
        if (!body.contains("clientId")) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson(
                    (nlohmann::json {{"title", "clientId is required"}})
                        .dump());
            return;
        }

        std::string clientId;

        if (body["clientId"].is_string()) {
            clientId = body["clientId"];
        } else if (body["clientId"].is_number()) {
            clientId = std::to_string(body["clientId"].get<int>());
        } else {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson((nlohmann::json {{"title",
                                                   "clientId must be a string "
                                                   "or a number"}})
                                     .dump());
            return;
        }

        this->PeerHangup(clientId);

        res->writeStatus(HTTP_200_OK)
            ->writeHeader("Content-Type", "application/json")
            ->end("{\"success\": true}");
    }

}  // namespace Web::Streaming::WebRTC