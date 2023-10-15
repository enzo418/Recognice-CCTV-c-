#pragma once

#include "ControllerUtils.hpp"
#include "DAL/IConfigurationDAO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Server/ServerContext.hpp"
#include "Streaming/PeerStreamingCapabilities.hpp"
#include "Streaming/StreamingServiceSelector.hpp"
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
        void StreamCamera(auto* res, auto* req, const nlohmann::json& body);

        /**
         * @brief Stream a source.
         * Receives a query parameter "uri" with the source uri.
         * Sends a SDP offer in json format, usable by RTCPeerConnection API.
         *
         * @param res
         * @param req
         */
        void StreamSource(auto* res, auto* req, const nlohmann::json& body);

        /**
         * @brief Stream the observer output.
         * Sends a SDP offer in json format, usable by RTCPeerConnection API.
         *
         * @param res
         * @param req
         */
        void StreamObserver(
            auto* res, auto* req,
            const Streaming::PeerStreamingCapabilities& peerCapabilities);

       private:
        Streaming::StreamingServiceSelector<SSL> streamingServices;
        Web::DAL::IConfigurationDAO* configurationDAO;
    };

    template <bool SSL>
    StreamingController<SSL>::StreamingController(
        uWS::App* app, Web::DAL::IConfigurationDAO* pConfigurationDAO,
        RecognizeContext* context)
        : streamingServices(app, context), configurationDAO(pConfigurationDAO) {
        app->post("/api/stream/camera/:id", [this](auto* res, auto* req) {
            READ_JSON_BODY(res, req, StreamCamera);
        });

        app->post("/api/stream/source", [this](auto* res, auto* req) {
            READ_JSON_BODY(res, req, StreamSource);
        });

        app->post("/api/stream/observer", [this](auto* res, auto* req) {
            READ_JSON_BODY_TYPED(res, req, StreamObserver,
                                 Streaming::PeerStreamingCapabilities);
        });
    }

    template <bool SSL>
    void StreamingController<SSL>::OnObserverStopped() {
        streamingServices.OnObserverStopped();
    }

    template <bool SSL>
    void StreamingController<SSL>::StreamCamera(auto* res, auto* req,
                                                const nlohmann::json& body) {
        auto id = std::string(req->getParameter(0));

        try {
            nldb::json camera = configurationDAO->GetCamera(id);

            Streaming::PeerStreamingCapabilities peerCapabilities = body;

            std::unordered_map<std::string, std::string> result;

            if (!streamingServices.PrepareStream(camera["url"],
                                                 peerCapabilities, result)) {
                res->writeStatus(HTTP_404_NOT_FOUND)
                    ->writeHeader("Content-Type", "application/json")
                    ->end("{\"error\": \"source not found\"}");
                return;
            }

            res->endJson(nlohmann::json(result).dump());
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Error while streaming camera: {}", e.what());

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson("{\"title\": \"camera not found\"}");
        }
    }

    template <bool SSL>
    void StreamingController<SSL>::StreamSource(auto* res, auto* req,
                                                const nlohmann::json& body) {
        std::string uri(req->getQuery("uri"));

        Streaming::PeerStreamingCapabilities peerCapabilities = body;

        std::unordered_map<std::string, std::string> result;
        try {
            if (!streamingServices.PrepareStream(uri, peerCapabilities,
                                                 result)) {
                res->writeStatus(HTTP_404_NOT_FOUND)
                    ->writeHeader("Content-Type", "application/json")
                    ->end("{\"error\": \"source not found\"}");
                return;
            }

            res->endJson(nlohmann::json(result).dump());
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Error while streaming source: {}", e.what());

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson("{\"title\": \"source not found\"}");
        }
    }

    template <bool SSL>
    void StreamingController<SSL>::StreamObserver(
        auto* res, auto* req,
        const Streaming::PeerStreamingCapabilities& peerCapabilities) {
        std::unordered_map<std::string, std::string> result;

        try {
            if (!streamingServices.PrepareStreamObserver(peerCapabilities,
                                                         result)) {
                res->writeStatus(HTTP_404_NOT_FOUND)
                    ->writeHeader("Content-Type", "application/json")
                    ->end("{\"error\": \"observer not found\"}");
                return;
            }

            res->endJson(nlohmann::json(result).dump());
        } catch (const std::exception& e) {
            OBSERVER_ERROR("Error while streaming observer: {}", e.what());

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson("{\"title\": \"observer not found\"}");
        }
    }
}  // namespace Web::Controller