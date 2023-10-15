#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Server/RecognizeContext.hpp"
#include "SocketData.hpp"
#include "Streaming/Video/CameraLiveVideo.hpp"
#include "Streaming/Video/ObserverLiveVideo.hpp"
#include "Streaming/ws/WebsocketService.hpp"

namespace Web::Streaming::Ws {
    using namespace Web::Streaming::Video;

    template <bool SSL>
    class LiveViewsManager final {
       private:
        typedef typename uWS::WebSocket<SSL, true, PerSocketData> Client;
        typedef StreamWriter<SSL, Client> SourceVideo;
        typedef WebsocketService<SSL, PerSocketData> Service;

       public:
        constexpr static const char* observerUri = "observer";
        constexpr static const char* observerFeedId = "feed_observer";

       public:
        LiveViewsManager(RecognizeContext* recognizeCtx,
                         int compressionQuality = 90);

        ~LiveViewsManager();

       public:
        void AddClient(Client* client);
        void RemoveClient(Client* client);

       public:
        /**
         * @brief Check if a feed id exists, if exists also check wether it's
         * initialized or not.
         *
         * @param feedID
         * @return true
         * @return false
         */
        bool Exists(const std::string& feedID);

        /**
         * @brief Get the live feed id for a camera uri, or observer if uri ==
         * `observerUri`
         *
         * @param uri
         * @throw ObserverNotRunningException in case recognizer is not running
         * @throw InvalidCameraUriException if camera uri is not valid
         * @return std::string_view feed id to request a ws connection
         */
        std::string_view GetFeedId(const std::string& uri);

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

        SourceVideo* GetLiveView(const std::string& feed_id);

        LiveViewStatus GetStatus(const std::string& feed_id);

       private:
        /**
         * @brief Creates a new feed if there is no one for `uri`. Else return
         * the feedid
         *
         * @param uri
         * @return std::string_view feed id
         */
        std::string_view CreateOrReturnFeed(const std::string& uri);

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
    LiveViewsManager<SSL>::LiveViewsManager(RecognizeContext* pRecognizeCtx,
                                            int pCompressionQuality)
        : recognizeCtx(pRecognizeCtx),
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
    void LiveViewsManager<SSL>::AddClient(Client* client) {
        LockGuard guard(mtxServices);
        LockGuard guard2(mtxCamerasLiveView);

        std::string feedId(client->getUserData()->pathSubscribed);
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
    }

    template <bool SSL>
    void LiveViewsManager<SSL>::RemoveClient(Client* client) {
        LockGuard guard(mtxServices);
        LockGuard guard2(mtxCamerasLiveView);

        std::string feedId(client->getUserData()->pathSubscribed);
        if (Exists(feedId)) {
            auto service = services[feedId];
            service->RemoveClient(client);

            if (service->GetTotalClients() == 0) {
                OBSERVER_TRACE("Stopping live view since there are no clients");
                auto feed = camerasLiveView[feedId];
                feed->Stop();
            }
        }
    }

    template <bool SSL>
    std::string_view LiveViewsManager<SSL>::GetFeedId(const std::string& uri) {
        if (!mapUriToFeed.contains(uri)) {
            return {"", 0};
        }

        return std::string_view(mapUriToFeed[uri]);
    }

    template <bool SSL>
    typename LiveViewsManager<SSL>::SourceVideo*
    LiveViewsManager<SSL>::GetLiveView(const std::string& feedId) {
        return camerasLiveView[feedId];
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::Exists(const std::string& feedID) {
        // map has that key and unique pointer is not empty
        return camerasLiveView.contains(feedID);
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
                std::nullopt, this->compressionQuality, service);

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
}  // namespace Web::Streaming::Ws