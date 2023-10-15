#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Server/RecognizeContext.hpp"
#include "SocketData.hpp"
#include "Streaming/IStreamingSetupService.hpp"
#include "Streaming/Video/CameraLiveVideo.hpp"
#include "Streaming/Video/ObserverLiveVideo.hpp"
#include "Streaming/http/HttpStreamingService.hpp"
#include "Streaming/ws/WebsocketService.hpp"
#include "nlohmann/json.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Streaming::Http {
    using namespace Web::Streaming::Video;

    template <bool SSL>
    class MJPEGStreaming final : public IStreamingSetupService {
       private:
        typedef typename uWS::HttpResponse<SSL> Client;
        typedef StreamWriter<SSL, Client> SourceVideo;
        typedef HttpStreamingService<SSL> Service;

       public:
        constexpr static const char* observerUri = "observer";
        constexpr static const char* observerFeedId = "feed_observer";

       public:
        MJPEGStreaming(uWS::App* app, RecognizeContext* recognizeCtx,
                       int compressionQuality = 90);

        ~MJPEGStreaming();

       public:
        bool PrepareStream(
            const std::string& uri,
            std::unordered_map<std::string, std::string>& result) override;

        bool PrepareStreamObserver(
            std::unordered_map<std::string, std::string>& result) override;

        void OnObserverStopped() override;

        bool StartMJPEGStreamToClient(std::string& streamId, Client* client);

       private:
        /**
         * @brief Create a camera view. Returns true if success.
         *
         * @param uri
         * @return true
         * @return false
         */
        bool CreateCameraView(const std::string& uri);

        /**
         * @brief Create a observer View object. Returns true if success.
         *
         * @param uri some random string, remember it to ask for the live view
         * later.
         * @return true
         * @return false
         */
        bool CreateObserverView(const std::string& uri);

        LiveViewStatus GetStatus(const std::string& feed_id);

        /**
         * @brief MJPEG Stream endpoint. This is the start of the stream once
         * the negotiation is done.
         * endpoint parameter:
         * - stream_id: the id of the stream to watch.
         *
         * @param res
         * @param req
         */
        void MJPEGStream(auto* res, auto* req);

       private:
        std::map<std::string, std::string> mapUriToFeed;
        std::map<std::string, Service*> services;
        std::map<std::string, SourceVideo*> camerasLiveView;

        std::mutex mtxServices;
        std::mutex mtxCamerasLiveView;

        typedef std::lock_guard<std::mutex> LockGuard;

       private:
        RecognizeContext* recognizeCtx;
        int compressionQuality;
    };

    template <bool SSL>
    MJPEGStreaming<SSL>::MJPEGStreaming(uWS::App* app,
                                        RecognizeContext* pRecognizeCtx,
                                        int pCompressionQuality)
        : recognizeCtx(pRecognizeCtx), compressionQuality(pCompressionQuality) {
        app->get("/live/mjpeg/stream/:stream_id*",
                 [this](auto* res, auto* req) { MJPEGStream(res, req); });
    }

    template <bool SSL>
    MJPEGStreaming<SSL>::~MJPEGStreaming() {
        for (auto& [k, v] : camerasLiveView) {
            delete v;
        }

        for (auto& [k, v] : services) {
            delete v;
        }
    }

    template <bool SSL>
    bool MJPEGStreaming<SSL>::PrepareStream(
        const std::string& uri,
        std::unordered_map<std::string, std::string>& result) {
        LockGuard guard(mtxServices);
        LockGuard guard2(mtxCamerasLiveView);

        if (mapUriToFeed.find(uri) == mapUriToFeed.end()) {
            if (!CreateCameraView(uri)) {
                return false;
            }
        }

        std::string feedId = mapUriToFeed[uri];

        auto service = services[feedId];
        auto feed = camerasLiveView[feedId];

        // check if it's running because it ONLY will change its status once the
        // thread is spawned.
        if (Observer::has_flag(feed->GetStatus(), LiveViewStatus::STOPPED) &&
            !feed->IsRunning()) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        }

        result["stream_url"] = "/live/mjpeg/stream/" + feedId;

        return true;
    }

    template <bool SSL>
    bool MJPEGStreaming<SSL>::PrepareStreamObserver(
        std::unordered_map<std::string, std::string>& result) {
        LockGuard guard(mtxServices);
        LockGuard guard2(mtxCamerasLiveView);

        if (!recognizeCtx->observer) {
            return false;
        }

        if (mapUriToFeed.find(observerUri) == mapUriToFeed.end()) {
            if (!CreateObserverView(observerUri)) {
                return false;
            }
        }

        std::string feedId = mapUriToFeed[observerUri];

        auto service = services[feedId];
        auto feed = camerasLiveView[feedId];

        // check if it's running because it ONLY will change its status once the
        // thread is spawned.
        if (Observer::has_flag(feed->GetStatus(), LiveViewStatus::STOPPED) &&
            !feed->IsRunning()) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        } else if (Observer::has_flag(feed->GetStatus(),
                                      LiveViewStatus::CLOSED) &&
                   feed->IsRunning()) {
            OBSERVER_TRACE("Restarting a zombie live feed: {}", feedId);
            feed->Stop();

            feed->Start();
        }

        result["stream_url"] = "/live/mjpeg/stream/" + feedId;

        return true;
    }

    template <bool SSL>
    bool MJPEGStreaming<SSL>::CreateCameraView(const std::string& uri) {
        if (mapUriToFeed.find(uri) != mapUriToFeed.end()) {
            return true;
        }

        std::string feedId = std::to_string(camerasLiveView.size());

        auto service = new Service();
        auto camera = new CameraLiveVideo<SSL, Client>(
            uri, this->compressionQuality, service);

        bool cameraDidOpen =
            !Observer::has_flag(camera->GetStatus(), LiveViewStatus::ERROR);

        if (cameraDidOpen) {
            services[feedId] = service;
            camerasLiveView[feedId] = camera;

            mapUriToFeed[uri] = std::move(feedId);
        } else {
            delete camera;
            delete service;
        }

        return cameraDidOpen;
    }

    template <bool SSL>
    bool MJPEGStreaming<SSL>::CreateObserverView(const std::string& uri) {
        if (!recognizeCtx->observer) {
            return false;
        }

        Service* service;

        // NOTE: Once observer was stopped, it will be removed from uri map but
        // the services, both client specific implementation and the observer
        // frame subscriber, will remain there.

        if (services.find(observerFeedId) == services.end()) {
            service = new Service();
            services[observerFeedId] = service;
        } else {
            service = services[observerFeedId];
        }

        if (camerasLiveView.find(observerFeedId) == camerasLiveView.end()) {
            camerasLiveView[observerFeedId] =
                new Video::Ws::ObserverLiveVideo<SSL, Client>(
                    std::nullopt, this->compressionQuality, service);
        }

        recognizeCtx->observer->SubscribeToFrames(
            (Video::Ws::ObserverLiveVideo<SSL, Client>*)
                camerasLiveView[observerFeedId]);

        mapUriToFeed[uri] = observerFeedId;

        return true;
    }

    template <bool SSL>
    LiveViewStatus MJPEGStreaming<SSL>::GetStatus(const std::string& feed_id) {
        return camerasLiveView[feed_id]->GetStatus();
    }

    template <bool SSL>
    void MJPEGStreaming<SSL>::OnObserverStopped() {
        mapUriToFeed.erase(observerUri);

        OBSERVER_INFO("Observer live view was stopped and cleaned up.");
    }

    template <bool SSL>
    bool MJPEGStreaming<SSL>::StartMJPEGStreamToClient(std::string& feedId,
                                                       Client* client) {
        LockGuard guard(mtxServices);
        LockGuard guard2(mtxCamerasLiveView);

        if (services.find(feedId) == services.end()) {
            return false;
        }

        auto service = services[feedId];
        service->AddClient(client);
        auto feed = camerasLiveView[feedId];

        // check if it's running because it ONLY will change its status once the
        // thread is spawned.
        if (Observer::has_flag(feed->GetStatus(), LiveViewStatus::STOPPED) &&
            !feed->IsRunning()) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        } else if (Observer::has_flag(feed->GetStatus(),
                                      LiveViewStatus::CLOSED) &&
                   feed->IsRunning()) {
            OBSERVER_TRACE("Restarting a zombie live feed: {}", feedId);
            feed->Stop();

            feed->Start();
        }

        return true;
    }

    template <bool SSL>
    void MJPEGStreaming<SSL>::MJPEGStream(auto* res, auto* req) {
        std::string streamId(req->getParameter(0));

        if (!StartMJPEGStreamToClient(streamId, res)) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Content-Type", "application/json")
                ->end("{\"error\": \"stream not found\"}");
            return;
        }
    }
}  // namespace Web::Streaming::Http