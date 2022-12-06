#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "../Server/RecognizeContext.hpp"
#include "CameraLiveVideo.hpp"
#include "LiveVideo.hpp"
#include "ObserverLiveVideo.hpp"

namespace Web {
    template <bool SSL>
    class LiveViewsManager {
       public:
        constexpr static const char* observerUri = "observer";
        constexpr static const char* observerFeedId = "feed_observer";

       public:
        LiveViewsManager(int observerFPS, RecognizeContext* recognizeCtx,
                         int compressionQuality = 90);

        ~LiveViewsManager();

       private:
        typedef typename LiveVideo<SSL>::WebSocketClient Client;

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
         * @param uri some random string, rembember it to ask for the live view
         * later.
         * @return true
         * @return false
         */
        bool CreateObserverView(const std::string& uri);

        LiveVideo<SSL>* GetLiveView(const std::string& feed_id);

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
        std::map<std::string, LiveVideo<SSL>*> camerasLiveView;

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
    }

    template <bool SSL>
    void LiveViewsManager<SSL>::AddClient(Client* client) {
        std::string feedId(client->getUserData()->pathSubscribed);
        auto feed = camerasLiveView[feedId];
        feed->AddClient(client);

        if (Observer::has_flag(feed->GetStatus(),
                               Web::LiveViewStatus::STOPPED)) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        }
    }

    template <bool SSL>
    void LiveViewsManager<SSL>::RemoveClient(Client* client) {
        std::string feedId(client->getUserData()->pathSubscribed);
        if (Exists(feedId)) {
            auto feed = camerasLiveView[feedId];

            feed->RemoveClient(client);

            if (feed->GetTotalClients() == 0) {
                feed->Stop();
                OBSERVER_TRACE("Stopping live view since there are no clients");
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
    LiveVideo<SSL>* LiveViewsManager<SSL>::GetLiveView(
        const std::string& feedId) {
        return camerasLiveView[feedId];
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::Exists(const std::string& feedID) {
        // map has that key and unique pointer is not empty
        return camerasLiveView.contains(feedID);
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::CreateCameraView(const std::string& uri) {
        std::string feedId = std::to_string(camerasLiveView.size());

        auto camera = new CameraLiveVideo<SSL>(uri, this->compressionQuality);

        bool cameraDidOpen =
            !Observer::has_flag(camera->GetStatus(), LiveViewStatus::ERROR);

        if (cameraDidOpen) {
            camerasLiveView[feedId] = camera;

            mapUriToFeed[uri] = std::move(feedId);
        } else {
            delete camera;
        }

        return cameraDidOpen;
    }

    template <bool SSL>
    bool LiveViewsManager<SSL>::CreateObserverView(const std::string& uri) {
        if (!recognizeCtx->observer) {
            return false;
        }

        camerasLiveView[observerFeedId] =
            new ObserverLiveVideo<SSL>(observerFPS, this->compressionQuality);

        recognizeCtx->observer->SubscribeToFrames(
            (ObserverLiveVideo<SSL>*)camerasLiveView[observerFeedId]);

        mapUriToFeed[uri] = observerFeedId;

        return true;
    }

    template <bool SSL>
    LiveViewStatus LiveViewsManager<SSL>::GetStatus(
        const std::string& feed_id) {
        return camerasLiveView[feed_id]->GetStatus();
    }
}  // namespace Web