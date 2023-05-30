#pragma once

#include <string>
#include <string_view>

#include "RecognizeContext.hpp"
#include "Streaming/http/LiveViewsManager.hpp"
#include "Streaming/ws/LiveViewsManager.hpp"

namespace Web {

    template <bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext recognizeContext;

        // std::unique_ptr<Streaming::Ws::LiveViewsManager<SSL>>
        // liveViewsManager;
        // std::unique_ptr<Streaming::Http::LiveViewsManager<SSL>>
        //     liveViewsManager;
    };
}  // namespace Web