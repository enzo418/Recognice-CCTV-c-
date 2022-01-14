#pragma once

#include <string>
#include <string_view>

#include "../LiveVideo/CameraLiveVideo.hpp"
#include "../LiveVideo/LiveViewsManager.hpp"
#include "../LiveVideo/ObserverLiveVideo.hpp"
#include "../Notifications/WebsocketNotificator.hpp"
#include "RecognizeContext.hpp"

namespace Web {

    template <typename TFrame, bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext<TFrame> recognizeContext;

        std::unique_ptr<LiveViewsManager<TFrame, SSL>> liveViewsManager;
        std::unique_ptr<WebsocketNotificator<SSL>> notificatorWS;
    };
}  // namespace Web