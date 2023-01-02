#pragma once

#include "Serialization/JsonSerialization.hpp"
#include "Server/ServerConfigurationProvider.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "Utils/JsonUtils.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {

    template <bool SSL>
    class LiveViewController {
       public:
        LiveViewController(uWS::App* app, Web::ServerContext<SSL>* serverCtx);

        void OnOpen(auto* ws, const std::list<std::string_view>& paths);
        void OnClose(auto* ws);

       private:
        Web::ServerContext<SSL>* serverCtx;
    };

    template <bool SSL>
    LiveViewController<SSL>::LiveViewController(
        uWS::App* app, Web::ServerContext<SSL>* pServerCtx)
        : serverCtx(pServerCtx) {
        app->ws<PerSocketData>(
            "/live/*",
            {.compression = uWS::DISABLED,
             .maxPayloadLength = 16 * 1024 * 1024,
             .idleTimeout = 16,
             .maxBackpressure = 1 * 1024 * 1024,
             .closeOnBackpressureLimit = false,
             .resetIdleTimeoutOnSend = false,
             .sendPingsAutomatically = true,
             .upgrade = nullptr,
             .open =
                 [this](auto* ws, const std::list<std::string_view>& paths) {
                     this->OnOpen(ws, paths);
                 },
             .close =
                 [this](auto* ws, int /*code*/, std::string_view /*message*/) {
                     this->OnClose(ws);
                 }});
    }

    template <bool SSL>
    void LiveViewController<SSL>::OnOpen(
        auto* ws, const std::list<std::string_view>& paths) {
        // 1° is live, 2° is *
        std::string feedID(*std::next(paths.begin()));
        ws->getUserData()->pathSubscribed = feedID;

        OBSERVER_INFO("Client connected to live '{0}'", feedID);

        if (serverCtx->liveViewsManager->Exists(feedID)) {
            serverCtx->liveViewsManager->AddClient(ws);
        } else {
            OBSERVER_ERROR("Live feed wasn't initialized yet! path: {0}",
                           feedID);

            ws->send(
                "Wrong feed id, this one doesn't exists! Closing connection.");
            ws->end();
        }
    }

    template <bool SSL>
    void LiveViewController<SSL>::OnClose(auto* ws) {
        OBSERVER_TRACE("Client disconnected from '{0}'",
                       ws->getUserData()->pathSubscribed);

        serverCtx->liveViewsManager->RemoveClient(ws);
    }
}  // namespace Web::Controller