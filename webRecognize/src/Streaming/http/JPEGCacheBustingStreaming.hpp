#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Server/RecognizeContext.hpp"
#include "SocketData.hpp"
#include "Streaming/IStreamingSetupService.hpp"
#include "Streaming/Video/CameraLiveVideo.hpp"
#include "Streaming/Video/CameraOnDemand.hpp"
#include "Streaming/Video/ObserverLiveVideo.hpp"
#include "Streaming/Video/ObserverOnDemand.hpp"
#include "Streaming/Video/SourceOnDemand.hpp"
#include "Streaming/http/HttpStreamingService.hpp"
#include "Streaming/ws/WebsocketService.hpp"
#include "nlohmann/json.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Streaming::Http {
    using namespace Web::Streaming::Video;

    template <bool SSL>
    class JPEGCacheBustingStreaming final : public IStreamingSetupService {
       public:
        constexpr static const char* observerFeedId = "feed_observer";

       public:
        JPEGCacheBustingStreaming(uWS::App* app, RecognizeContext* recognizeCtx,
                                  int compressionQuality = 90);

        ~JPEGCacheBustingStreaming();

       public:
        bool PrepareStream(
            const std::string& uri,
            std::unordered_map<std::string, std::string>& result) override;

        bool PrepareStreamObserver(
            std::unordered_map<std::string, std::string>& result) override;

        void OnObserverStopped() override;

       private:
        void OnImageRequest(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req);

        bool CreateCameraView(const std::string& uri);

       private:
        int compressionQuality;

        std::mutex mtxUriMap;
        std::map<std::string, std::string> mapUriToFeed;

        std::mutex mtxSources;
        std::map<std::string, SourceOnDemand*> sources;

        RecognizeContext* recognizeCtx;
    };

    template <bool SSL>
    JPEGCacheBustingStreaming<SSL>::JPEGCacheBustingStreaming(
        uWS::App* app, RecognizeContext* recognizeCtx, int compressionQuality)
        : recognizeCtx(recognizeCtx), compressionQuality(compressionQuality) {
        app->get("/live/jpg/stream/:stream_id*",
                 [this](auto* res, auto* req) { OnImageRequest(res, req); });
    }

    template <bool SSL>
    JPEGCacheBustingStreaming<SSL>::~JPEGCacheBustingStreaming() {
        for (auto source : sources) {
            delete source.second;
        }
    }

    template <bool SSL>
    bool JPEGCacheBustingStreaming<SSL>::CreateCameraView(
        const std::string& uri) {
        std::lock_guard<std::mutex> lock(mtxUriMap);

        if (mapUriToFeed.find(uri) != mapUriToFeed.end()) {
            return true;
        }

        std::string streamId = std::to_string(mapUriToFeed.size());

        CameraOnDemand* camera = new CameraOnDemand(uri, 90);

        bool cameraDidOpen =
            !Observer::has_flag(camera->GetStatus(), LiveViewStatus::ERROR);

        if (cameraDidOpen) {
            std::lock_guard<std::mutex> lock(mtxSources);
            sources[streamId] = camera;
            mapUriToFeed[uri] = std::move(streamId);
        } else {
            delete camera;
        }

        return cameraDidOpen;
    }

    template <bool SSL>
    bool JPEGCacheBustingStreaming<SSL>::PrepareStream(
        const std::string& uri,
        std::unordered_map<std::string, std::string>& result) {
        mtxUriMap.lock();
        bool alreadyExists = mapUriToFeed.find(uri) != mapUriToFeed.end();
        mtxUriMap.unlock();

        if (!alreadyExists) {
            if (!CreateCameraView(uri)) {
                return false;
            }
        }

        std::lock_guard<std::mutex> lock1(mtxUriMap);
        std::lock_guard<std::mutex> lock2(mtxSources);
        std::string streamId = mapUriToFeed[uri];
        CameraOnDemand* feed = (CameraOnDemand*)sources[streamId];

        feed->EnsureOpenAndReady();

        result["stream_url"] = "/live/jpg/stream/" + streamId;

        return true;
    }

    template <bool SSL>
    void JPEGCacheBustingStreaming<SSL>::OnObserverStopped() {
        OBSERVER_INFO("Stopping live view of observer");

        std::lock_guard<std::mutex> lock(mtxSources);
        if (sources.find(observerFeedId) != sources.end()) {
            ObserverOnDemand* feed =
                (ObserverOnDemand*)sources.at(observerFeedId);
            feed->Stop();
            delete feed;
            sources.erase(observerFeedId);
        }
    }

    template <bool SSL>
    bool JPEGCacheBustingStreaming<SSL>::PrepareStreamObserver(
        std::unordered_map<std::string, std::string>& result) {
        if (!recognizeCtx->observer) {
            return false;
        }

        std::lock_guard<std::mutex> lock(mtxSources);

        ObserverOnDemand* feed;

        if (sources.find(observerFeedId) == sources.end()) {
            feed = new ObserverOnDemand(90);

            sources[observerFeedId] = feed;

            recognizeCtx->observer->SubscribeToFrames((ObserverOnDemand*)feed);
        } else {
            feed = (ObserverOnDemand*)sources[observerFeedId];
        }

        feed->EnsureOpenAndReady();

        result["stream_url"] =
            std::string("/live/jpg/stream/") + observerFeedId;

        return true;
    }

    template <bool SSL>
    void JPEGCacheBustingStreaming<SSL>::OnImageRequest(
        uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req) {
        std::string_view streamId = req->getParameter(0);

        auto feed_it = sources.find(std::string(streamId));

        if (feed_it == sources.end()) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control",
                              "no-cache, no-store, must-revalidate")
                ->end();
            return;
        }

        SourceOnDemand* feed = feed_it->second;

        feed->EnsureOpenAndReady();

        const bool hadFrame = feed->RunSafelyOnFrameReady(
            [res](const CameraOnDemand::Buffer& buffer) {
                res->writeHeader("Content-Type", "image/jpeg");
                res->writeHeader("Cache-Control",
                                 "no-cache, no-store, must-revalidate");
                res->writeHeader("Pragma", "no-cache");
                res->writeHeader("Expires", "0");
                res->end(std::string_view((const char*)buffer.data(),
                                          buffer.size()));
            });

        if (!hadFrame) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control",
                              "no-cache, no-store, must-revalidate")
                ->end();
        }
    }

}  // namespace Web::Streaming::Http