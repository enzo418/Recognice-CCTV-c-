#pragma once

#include "ControllerUtils.hpp"
#include "DAL/IConfigurationDAO.hpp"
#include "Server/ServerContext.hpp"
#include "Streaming/WebRTC/WebRTCStreaming.hpp"
#include "Streaming/http/LiveViewsManager.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class StreamingController {
       public:
        StreamingController(uWS::App* app,
                            Web::DAL::IConfigurationDAO* configurationDAO,
                            RecognizeContext* context);

        /**
         * @brief Called when observer was stopped.
         *
         */
        void OnObserverStopped();

       private:
        /**
         * @brief Stream a camera.
         * Receives a query parameter "id" with the camera id.
         *
         * @param res
         * @param req
         */
        void StreamCamera(auto* res, auto* req);

        /**
         * @brief Stream a source.
         * Receives a query parameter "uri" with the source uri.
         * Sends a SDP offer in json format, usable by RTCPeerConnection API.
         *
         * @param res
         * @param req
         */
        void StreamSource(auto* res, auto* req);

        /**
         * @brief Stream the observer output.
         * Sends a SDP offer in json format, usable by RTCPeerConnection API.
         *
         * @param res
         * @param req
         */
        void StreamObserver(auto* res, auto* req);

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
        Streaming::WebRTC::WebRTCStreamingService<SSL> streamingServices;
        Web::DAL::IConfigurationDAO* configurationDAO;
    };

    template <bool SSL>
    StreamingController<SSL>::StreamingController(
        uWS::App* app, Web::DAL::IConfigurationDAO* pConfigurationDAO,
        RecognizeContext* context)
        : streamingServices(context), configurationDAO(pConfigurationDAO) {
        app->get("/api/stream/camera/:id", [this](auto* res, auto* req) {
            this->StreamCamera(res, req);
        });

        app->get("/api/stream/source", [this](auto* res, auto* req) {
            this->StreamSource(res, req);
        });

        app->get("/api/stream/observer", [this](auto* res, auto* req) {
            this->StreamObserver(res, req);
        });

        // It should be POST /api/stream/:client_id/answer but uWebSockets has a
        // hard time parsing the numeric parameters...
        app->post("/api/stream/answer", [this](auto* res, auto* req) {
            READ_JSON_BODY(res, req, SetPeerAnswer);
        });

        // I would like to use DELETE method, /api/stream/:client_id
        app->post("/api/stream/hangup", [this](auto* res, auto* req) {
            READ_JSON_BODY(res, req, PeerHangup);
        });
    }

    template <bool SSL>
    void StreamingController<SSL>::OnObserverStopped() {
        streamingServices.OnObserverStopped();
    }

    template <bool SSL>
    void StreamingController<SSL>::StreamCamera(auto* res, auto* req) {
        auto id = std::string(req->getParameter(0));

        try {
            nldb::json camera = configurationDAO->GetCamera(id);

            if (!streamingServices.Stream(camera["url"], res)) {
                res->writeStatus(HTTP_404_NOT_FOUND)
                    ->writeHeader("Content-Type", "application/json")
                    ->end("{\"error\": \"source not found\"}");
                return;
            }
        } catch (const std::exception& e) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson("{\"title\": \"camera not found\"}");
        }
    }

    template <bool SSL>
    void StreamingController<SSL>::StreamSource(auto* res, auto* req) {
        std::string uri(req->getQuery("uri"));
        if (!streamingServices.Stream(uri, res)) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"source not found\"}");
            return;
        }
    }

    template <bool SSL>
    void StreamingController<SSL>::StreamObserver(auto* res, auto* req) {
        if (!streamingServices.StreamObserver(res)) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"observer not found\"}");
            return;
        }
    }

    template <bool SSL>
    void StreamingController<SSL>::SetPeerAnswer(auto* res, auto* req,
                                                 const nlohmann::json& body) {
        if (!body.contains("clientId") || !body.contains("answer")) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"clientId and answer are required\"}");
            return;
        }

        std::string clientId;

        if (body["clientId"].is_string()) {
            clientId = body["clientId"];
        } else if (body["clientId"].is_number()) {
            clientId = std::to_string(body["clientId"].get<int>());
        } else {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"clientId must be a string or a number\"}");
            return;
        }

        std::string answer;

        if (body["answer"].is_string()) {
            answer = body["answer"];
        } else {
            answer = body["answer"].dump();
        }

        streamingServices.SetPeerAnswer(clientId, answer);

        res->writeStatus(HTTP_200_OK)
            ->writeHeader("Content-Type", "application/json")
            ->end("{\"success\": true}");
    }

    template <bool SSL>
    void StreamingController<SSL>::PeerHangup(auto* res, auto* req,
                                              const nlohmann::json& body) {
        if (!body.contains("clientId")) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"clientId is required\"}");
            return;
        }

        std::string clientId;

        if (body["clientId"].is_string()) {
            clientId = body["clientId"];
        } else if (body["clientId"].is_number()) {
            clientId = std::to_string(body["clientId"].get<int>());
        } else {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"clientId must be a string or a number\"}");
            return;
        }

        streamingServices.PeerHangup(clientId);

        res->writeStatus(HTTP_200_OK)
            ->writeHeader("Content-Type", "application/json")
            ->end("{\"success\": true}");
    }
}  // namespace Web::Controller