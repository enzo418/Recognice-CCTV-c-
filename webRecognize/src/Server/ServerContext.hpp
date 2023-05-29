#pragma once

#include <string>
#include <string_view>

#include "RecognizeContext.hpp"
#include "Streaming/Video/ws/LiveViewsManager.hpp"

namespace Web {

    template <bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext recognizeContext;

        std::unique_ptr<Streaming::Video::Ws::LiveViewsManager<SSL>>
            liveViewsManager;
    };
}  // namespace Web