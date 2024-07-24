#pragma once

#include <functional>
#include <future>

#include "DAL/IConfigurationDAO.hpp"
#include "DTO/ObserverStatusDTO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Server/RecognizeContext.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "Streaming/ws/WebsocketService.hpp"
#include "Utils/ObserverStatus.hpp"
#include "observer/Log/log.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/Loop.h"

namespace Web::Controller {
    template <bool SSL>
    class ObserverController {
       public:
        ObserverController(
            uWS::App* app, Web::ServerContext<SSL>* serverCtx,
            Web::DAL::IConfigurationDAO* configurationDAO,
            std::function<void(Observer::Configuration& cfg,
                               std::function<void()> onStarted)>&&
                startRecognizeFunction,
            std::function<void()>&& stopRecognizeFunction,
            Web::Streaming::Ws::WebsocketService<SSL, PerSocketData>*
                wsService);

        void Start(auto* res, auto* req);
        void Stop(auto* res, auto* req);
        void Status(auto* res, auto* req);
        void RequestStream(auto* res, auto* req);

       private:
        Web::ServerContext<SSL>* serverCtx;
        Web::DAL::IConfigurationDAO* configurationDAO;

        std::function<void(Observer::Configuration& cfg, std::function<void()>)>
            startRecognize;
        std::function<void()> stopRecognize;

        std::list<std::future<void>> asyncTasks;
        uWS::Loop* loop;

       private:
        Streaming::Ws::WebsocketService<SSL, PerSocketData>* statusWsService;
    };

    template <bool SSL>
    ObserverController<SSL>::ObserverController(
        uWS::App* app, Web::ServerContext<SSL>* pServerCtx,
        Web::DAL::IConfigurationDAO* pConfigurationDAO,
        std::function<void(Observer::Configuration& cfg,
                           std::function<void()>)>&& pStartRecognizeFunction,
        std::function<void()>&& pStopRecognizeFunction,
        Web::Streaming::Ws::WebsocketService<SSL, PerSocketData>* wsService)
        : serverCtx(pServerCtx),
          configurationDAO(pConfigurationDAO),
          startRecognize(std::move(pStartRecognizeFunction)),
          stopRecognize(std::move(pStopRecognizeFunction)),
          statusWsService(wsService) {
        app->get("/api/observer/start/:config_id",
                 [this](auto* res, auto* req) { this->Start(res, req); });

        app->get("/api/observer/stop",
                 [this](auto* res, auto* req) { this->Stop(res, req); });

        app->get("/api/observer/status",
                 [this](auto* res, auto* req) { this->Status(res, req); });

        app->ws<PerSocketData>(
            "/observer/status",
            {.open =
                 [this](auto* ws, const std::list<std::string_view>& paths) {
                     this->statusWsService->AddClient(ws);

                     auto statusString = Utils::GetStatusJsonString(
                         this->serverCtx->recognizeContext);

                     ws->send(statusString);
                 },
             .close =
                 [this](auto* ws, int /*code*/, std::string_view /*message*/) {
                     this->statusWsService->RemoveClient(ws);
                 }});

        loop = uWS::Loop::get();
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

        try {
            res->onAborted([]() {});

            startRecognize(cfg, [this, res]() {
                loop->defer([this, res]() {
                    OBSERVER_TRACE("Observer started");

                    auto statusString = Utils::GetStatusJsonString(
                        this->serverCtx->recognizeContext);

                    res->end(statusString);

                    statusWsService->SendToClients(statusString.c_str(),
                                                   statusString.size());
                });
            });
        } catch (const std::exception& e) {
            OBSERVER_WARN("Error starting observer: {}", e.what());

            nlohmann::json response = {{"title", "Error starting observer"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
            return;
        }
    }

    template <bool SSL>
    void ObserverController<SSL>::Stop(auto* res, auto* req) {
        res->onAborted([]() {});

        asyncTasks.push_back(std::async(std::launch::async, [this, res]() {
            if (serverCtx->recognizeContext.running) {
                stopRecognize();
            }

            auto statusString =
                Utils::GetStatusJsonString(this->serverCtx->recognizeContext);

            loop->defer([this, res, statusString]() {
                res->end(statusString);

                statusWsService->SendToClients(statusString.c_str(),
                                               statusString.size());
            });
        }));
    }

    template <bool SSL>
    void ObserverController<SSL>::Status(auto* res, auto* req) {
        res->writeHeader("Cache-Control", "max-age=5")
            ->end(
                Utils::GetStatusJsonString(this->serverCtx->recognizeContext));
    }
}  // namespace Web::Controller