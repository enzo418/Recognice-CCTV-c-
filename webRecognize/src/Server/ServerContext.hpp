#pragma once

#include <string>
#include <string_view>

#include "../stream_content/LiveVideo/CameraLiveVideo.hpp"
#include "../stream_content/LiveVideo/ObserverLiveVideo.hpp"
#include "RecognizeContext.hpp"

namespace Web {

    template <typename TFrame, bool SSL>
    struct LiveCamerasContext {
        std::map<std::string, std::string> mapUriToFeed;

        std::map<std::string, std::unique_ptr<CameraLiveVideo<TFrame, SSL>>>
            camerasLiveView;
    };

    template <typename TFrame, bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext<TFrame> recognizeContext;

        std::unique_ptr<ObserverLiveVideo<TFrame, SSL>> observerLiveView;
        std::unique_ptr<LiveCamerasContext<TFrame, SSL>> liveCamerasContext;
    };
}  // namespace Web