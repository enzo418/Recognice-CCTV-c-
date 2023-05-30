#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Server/RecognizeContext.hpp"
#include "SocketData.hpp"
#include "Streaming/Video/CameraLiveVideo.hpp"
#include "Streaming/Video/ObserverLiveVideo.hpp"
#include "Streaming/http/HttpStreamingService.hpp"
#include "Streaming/ws/WebsocketService.hpp"

namespace Web::Streaming::Http {
    using namespace Web::Streaming::Video;

    template <bool SSL>
    class LiveViewsManager final {
       private:
        typedef typename uWS::HttpResponse<SSL> Client;
        typedef StreamWriter<SSL, Client> SourceVideo;
        typedef HttpStreamingService<SSL> Service;

       public:
        constexpr static const char* observerUri = "observer";
        constexpr static const char* observerFeedId = "feed_observer";

       public:
        LiveViewsManager(int observerFPS, RecognizeContext* recognizeCtx,
                         int compressionQuality = 90);

        ~LiveViewsManager();

       public:
        /**
         * @brief Streams a source video to a client.
         *
         * @param uri url, uri, file path or reachable stream.
         * @param client http client
         * @return true if success
         */
        bool Stream(const std::string& uri, Client* client);

        /**
         * @brief Streams the observer output to a client.
         *
         * @param client
         * @return true if success
         */
        bool StreamObserver(Client* client);

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
        int observerFPS;
    };

    template <bool SSL>
    LiveViewsManager<SSL>::LiveViewsManager(int pObserverFPS,
                                            RecognizeContext* pRecognizeCtx,
                                            int pCompressionQuality)
        : observerFPS(pObserverFPS),
          recognizeCtx(pRecognizeCtx),
          compressionQuality(pCompressionQuality) {}

    template <bool SSL>
    LiveViewsManager<SSL>::~LiveViewsManager() {
        for (auto& [k, v] : camerasLiveView) {
            delete v;
        }

        for (auto& [k, v] : services) {
            delete v;
        }
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::Stream(const std::string& uri, Client* client) {
        LockGuard guard(mtxServices);
        LockGuard guard2(mtxCamerasLiveView);

        if (mapUriToFeed.find(uri) == mapUriToFeed.end()) {
            if (!CreateCameraView(uri)) {
                return false;
            }
        }

        std::string feedId = mapUriToFeed[uri];

        auto service = services[feedId];
        service->AddClient(client);
        auto feed = camerasLiveView[feedId];

        // check if it's running because it ONLY will change its status once the
        // thread is spawned.
        if (Observer::has_flag(feed->GetStatus(), LiveViewStatus::STOPPED) &&
            !feed->IsRunning()) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        }

        return true;
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::StreamObserver(Client* client) {
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
        service->AddClient(client);
        auto feed = camerasLiveView[feedId];

        // check if it's running because it ONLY will change its status once the
        // thread is spawned.
        if (Observer::has_flag(feed->GetStatus(), LiveViewStatus::STOPPED) &&
            !feed->IsRunning()) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        }

        return true;
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::CreateCameraView(const std::string& uri) {
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
    bool LiveViewsManager<SSL>::CreateObserverView(const std::string& uri) {
        if (!recognizeCtx->observer) {
            return false;
        }

        auto service = new Service();

        services[observerFeedId] = service;
        camerasLiveView[observerFeedId] =
            new Video::Ws::ObserverLiveVideo<SSL, Client>(
                observerFPS, this->compressionQuality, service);

        recognizeCtx->observer->SubscribeToFrames(
            (Video::Ws::ObserverLiveVideo<SSL, Client>*)
                camerasLiveView[observerFeedId]);

        mapUriToFeed[uri] = observerFeedId;

        return true;
    }

    template <bool SSL>
    LiveViewStatus LiveViewsManager<SSL>::GetStatus(
        const std::string& feed_id) {
        return camerasLiveView[feed_id]->GetStatus();
    }
}  // namespace Web::Streaming::Http