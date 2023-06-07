#pragma once

#include "DAL/IConfigurationDAO.hpp"
#include "Server/ServerContext.hpp"
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
        void StreamCamera(auto* res, auto* req);
        void StreamSource(auto* res, auto* req);
        void StreamObserver(auto* res, auto* req);
        void StreamDetectionConfiguration(auto* res, auto* req);

       private:
        Streaming::Http::LiveViewsManager<SSL> streamingServices;
        Web::DAL::IConfigurationDAO* configurationDAO;
    };

    template <bool SSL>
    StreamingController<SSL>::StreamingController(
        uWS::App* app, Web::DAL::IConfigurationDAO* pConfigurationDAO,
        RecognizeContext* context)
        : streamingServices(20, context), configurationDAO(pConfigurationDAO) {
        app->get("/api/stream/camera/:id", [this](auto* res, auto* req) {
            this->StreamCamera(res, req);
        });

        app->get("/api/stream/source", [this](auto* res, auto* req) {
            this->StreamSource(res, req);
        });

        app->get("/api/stream/observer", [this](auto* res, auto* req) {
            this->StreamObserver(res, req);
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
            nlohmann::json response = {{"title", "camera not found"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
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
}  // namespace Web::Controller