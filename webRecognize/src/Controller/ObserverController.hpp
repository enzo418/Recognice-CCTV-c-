#pragma once

#include <functional>

#include "DAL/ConfigurationDAO.hpp"
#include "DTO/ObserverStatusDTO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Server/RecognizeContext.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class ObserverController {
       public:
        ObserverController(
            uWS::App* app, Web::RecognizeContext* observerCtx,
            Web::DAL::ConfigurationDAO* configurationDAO,
            std::function<void(const Observer::Configuration& cfg)>&&
                startRecognizeFunction,
            std::function<void()>&& stopRecognizeFunction);

        void Start(auto* res, auto* req);
        void Stop(auto* res, auto* req);
        void Status(auto* res, auto* req);

       private:
        Web::RecognizeContext* observerCtx;
        Web::DAL::ConfigurationDAO* configurationDAO;

        std::function<void(const Observer::Configuration& cfg)> startRecognize;
        std::function<void()> stopRecognize;
    };

    template <bool SSL>
    ObserverController<SSL>::ObserverController(
        uWS::App* app, Web::RecognizeContext* pObserverCtx,
        Web::DAL::ConfigurationDAO* pConfigurationDAO,
        std::function<void(const Observer::Configuration& cfg)>&&
            pStartRecognizeFunction,
        std::function<void()>&& pStopRecognizeFunction)
        : observerCtx(pObserverCtx),
          configurationDAO(pConfigurationDAO),
          startRecognize(std::move(pStartRecognizeFunction)),
          stopRecognize(std::move(pStopRecognizeFunction)) {
        app->get("/api/start/:config_id",
                 [this](auto* res, auto* req) { this->Start(res, req); });

        app->get("/api/stop",
                 [this](auto* res, auto* req) { this->Stop(res, req); });

        app->get("/api/observerStatus",
                 [this](auto* res, auto* req) { this->Status(res, req); });
    }

    template <bool SSL>
    void ObserverController<SSL>::Start(auto* res, auto* req) {
        if (observerCtx->running) {
            nlohmann::json response = {
                {"title", "Observer is already running"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
            return;
        }

        auto id = req->getParameter(0);
        observerCtx->running_config_id = std::string(id);

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
            nlohmann::json response = {{"title", "Not a valid configuration"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
            return;
        }

        startRecognize(cfg);

        res->end(
            nlohmann::json(Web::ObserverStatusDTO {
                               .running = true,
                               .config_id = observerCtx->running_config_id})
                .dump());
    }

    template <bool SSL>
    void ObserverController<SSL>::Stop(auto* res, auto* req) {
        if (observerCtx->running) {
            stopRecognize();
        }

        res->end(
            nlohmann::json(Web::ObserverStatusDTO {.running = false,
                                                   .config_id = std::nullopt})
                .dump());
    }

    template <bool SSL>
    void ObserverController<SSL>::Status(auto* res, auto* req) {
        bool running = observerCtx->running;
        std::optional<std::string> cfg_id;
        if (running) cfg_id = observerCtx->running_config_id;

        res->end(nlohmann::json(Web::ObserverStatusDTO {.running = running,
                                                        .config_id = cfg_id})
                     .dump());
    }

}  // namespace Web::Controller