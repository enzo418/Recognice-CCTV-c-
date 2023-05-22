#pragma once

#include <functional>

#include "DAL/IConfigurationDAO.hpp"
#include "DTO/ObserverStatusDTO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Server/RecognizeContext.hpp"
#include "Server/ServerContext.hpp"
#include "observer/Log/log.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class ObserverController {
       public:
        ObserverController(uWS::App* app, Web::ServerContext<SSL>* serverCtx,
                           Web::DAL::IConfigurationDAO* configurationDAO,
                           std::function<void(Observer::Configuration& cfg)>&&
                               startRecognizeFunction,
                           std::function<void()>&& stopRecognizeFunction);

        void Start(auto* res, auto* req);
        void Stop(auto* res, auto* req);
        void Status(auto* res, auto* req);
        void RequestStream(auto* res, auto* req);

       private:
        Web::ServerContext<SSL>* serverCtx;
        Web::DAL::IConfigurationDAO* configurationDAO;

        std::function<void(Observer::Configuration& cfg)> startRecognize;
        std::function<void()> stopRecognize;
    };

    template <bool SSL>
    ObserverController<SSL>::ObserverController(
        uWS::App* app, Web::ServerContext<SSL>* pServerCtx,
        Web::DAL::IConfigurationDAO* pConfigurationDAO,
        std::function<void(Observer::Configuration& cfg)>&&
            pStartRecognizeFunction,
        std::function<void()>&& pStopRecognizeFunction)
        : serverCtx(pServerCtx),
          configurationDAO(pConfigurationDAO),
          startRecognize(std::move(pStartRecognizeFunction)),
          stopRecognize(std::move(pStopRecognizeFunction)) {
        app->get("/api/observer/start/:config_id",
                 [this](auto* res, auto* req) { this->Start(res, req); });

        app->get("/api/observer/stop",
                 [this](auto* res, auto* req) { this->Stop(res, req); });

        app->get("/api/observer/status",
                 [this](auto* res, auto* req) { this->Status(res, req); });

        app->get("/api/observer/stream", [this](auto* res, auto* req) {
            this->RequestStream(res, req);
        });
    }

    template <bool SSL>
    void ObserverController<SSL>::Start(auto* res, auto* req) {
        if (serverCtx->recognizeContext.running) {
            nlohmann::json response = {
                {"title", "Observer is already running"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
            return;
        }

        auto id = req->getParameter(0);
        serverCtx->recognizeContext.running_config_id = std::string(id);

        nldb::json obj;
        try {
            obj = configurationDAO->GetConfiguration(std::string(id));
        } catch (const std::exception& e) {
            nlohmann::json response = {{"title", "Configuration not found"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
            return;
        }

        Observer::Configuration cfg;
        try {
            cfg = obj;
        } catch (const std::exception& e) {
            OBSERVER_WARN("Invalid configuration: {}", e.what());

            nlohmann::json response = {{"title", "Not a valid configuration"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
            return;
        }

        startRecognize(cfg);

        res->end(
            nlohmann::json(
                Web::ObserverStatusDTO {
                    .running = true,
                    .config_id = serverCtx->recognizeContext.running_config_id})
                .dump());
    }

    template <bool SSL>
    void ObserverController<SSL>::Stop(auto* res, auto* req) {
        if (serverCtx->recognizeContext.running) {
            stopRecognize();
        }

        res->end(
            nlohmann::json(Web::ObserverStatusDTO {.running = false,
                                                   .config_id = std::nullopt})
                .dump());
    }

    template <bool SSL>
    void ObserverController<SSL>::Status(auto* res, auto* req) {
        bool running = serverCtx->recognizeContext.running;
        std::optional<std::string> cfg_id;
        if (running) cfg_id = serverCtx->recognizeContext.running_config_id;

        res->end(nlohmann::json(Web::ObserverStatusDTO {.running = running,
                                                        .config_id = cfg_id})
                     .dump());
    }

    template <bool SSL>
    void ObserverController<SSL>::RequestStream(auto* res, auto* req) {
        auto uri = Web::LiveViewsManager<SSL>::observerUri;

        if (serverCtx->recognizeContext.running &&
            serverCtx->liveViewsManager->CreateObserverView(uri)) {
            std::string feed_id(serverCtx->liveViewsManager->GetFeedId(uri));

            nlohmann::json response = {{"ws_feed_id", feed_id}};

            res->endJson(response.dump());
        } else {
            nlohmann::json error = {{"title", "Observer is not running."}};

            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson(error.dump());
        }
    }
}  // namespace Web::Controller