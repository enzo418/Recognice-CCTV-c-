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
    template <typename TFrame, bool SSL>
    class LiveViewsManager {
       public:
        constexpr static const char* observerUri = "observer";
        constexpr static const char* observerFeedId = "feed_observer";

       public:
        LiveViewsManager(int observerFPS,
                         RecognizeContext<TFrame>* recognizeCtx,
                         int compressionQuality = 90);

        ~LiveViewsManager();

       private:
        typedef typename LiveVideo<TFrame, SSL>::WebSocketClient Client;

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

        LiveVideo<TFrame, SSL>* GetLiveView(const std::string& feed_id);

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
        std::map<std::string, LiveVideo<TFrame, SSL>*> camerasLiveView;

       private:
        RecognizeContext<TFrame>* recognizeCtx;
        int compressionQuality;
        int observerFPS;
    };

    template <typename TFrame, bool SSL>
    LiveViewsManager<TFrame, SSL>::LiveViewsManager(
        int pObserverFPS, RecognizeContext<TFrame>* pRecognizeCtx,
        int pCompressionQuality)
        : observerFPS(pObserverFPS),
          recognizeCtx(pRecognizeCtx),
          compressionQuality(pCompressionQuality) {}

    template <typename TFrame, bool SSL>
    LiveViewsManager<TFrame, SSL>::~LiveViewsManager() {
        for (auto& [k, v] : camerasLiveView) {
            delete v;
        }
    }

    template <typename TFrame, bool SSL>
    void LiveViewsManager<TFrame, SSL>::AddClient(Client* client) {
        std::string feedId(client->getUserData()->pathSubscribed);
        auto feed = camerasLiveView[feedId];
        feed->AddClient(client);

        if (Observer::has_flag(feed->GetStatus(),
                               Web::LiveViewStatus::STOPPED)) {
            OBSERVER_TRACE("Starting a live feed: {}", feedId);
            feed->Start();
        }
    }

    template <typename TFrame, bool SSL>
    void LiveViewsManager<TFrame, SSL>::RemoveClient(Client* client) {
        std::string feedId(client->getUserData()->pathSubscribed);
        auto feed = camerasLiveView[feedId];

        feed->RemoveClient(client);

        if (feed->GetTotalClients() == 0) {
            feed->Stop();
            OBSERVER_TRACE("Stopping live view since there are no clients");
        }
    }

    template <typename TFrame, bool SSL>
    std::string_view LiveViewsManager<TFrame, SSL>::GetFeedId(
        const std::string& uri) {
        if (uri == observerUri && !recognizeCtx->running) {
            throw ObserverNotRunningException();
        }

        return this->CreateOrReturnFeed(uri);
    }

    template <typename TFrame, bool SSL>
    LiveVideo<TFrame, SSL>* LiveViewsManager<TFrame, SSL>::GetLiveView(
        const std::string& feedId) {
        return camerasLiveView[feedId];
    }

    template <typename TFrame, bool SSL>
    bool LiveViewsManager<TFrame, SSL>::Exists(const std::string& feedID) {
        // map has that key and unique pointer is not empty
        return camerasLiveView.contains(feedID);
    }

    template <typename TFrame, bool SSL>
    std::string_view LiveViewsManager<TFrame, SSL>::CreateOrReturnFeed(
        const std::string& uri) {
        if (!mapUriToFeed.contains(uri)) {
            if (uri == observerUri) {
                camerasLiveView[observerFeedId] =
                    new ObserverLiveVideo<TFrame, SSL>(
                        observerFPS, this->compressionQuality);

                recognizeCtx->observer->SubscribeToFrames(
                    (ObserverLiveVideo<TFrame, SSL>*)
                        camerasLiveView[observerFeedId]);

                mapUriToFeed[uri] = observerFeedId;
            } else {
                std::string feedId = std::to_string(camerasLiveView.size());

                // calling the constructor will throw an
                // InvalidCameraUriException if the camera uri is invalid,
                // do not catch it
                camerasLiveView[feedId] = new CameraLiveVideo<TFrame, SSL>(
                    uri, this->compressionQuality);

                mapUriToFeed[uri] = std::move(feedId);
            }
        }

        return std::string_view(mapUriToFeed[uri]);
    }
}  // namespace Web