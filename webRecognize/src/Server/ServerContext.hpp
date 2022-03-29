#pragma once

#include <string>
#include <string_view>

#include "../LiveVideo/CameraLiveVideo.hpp"
#include "../LiveVideo/LiveViewsManager.hpp"
#include "../LiveVideo/ObserverLiveVideo.hpp"
#include "../Notifications/WebsocketNotificator.hpp"
#include "RecognizeContext.hpp"

namespace Web {

    template <bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext recognizeContext;

        std::unique_ptr<LiveViewsManager<SSL>> liveViewsManager;
    };
}  // namespace Web