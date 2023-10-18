#pragma once

#include <string>
#include <unordered_map>

namespace Web::Streaming {
    class IStreamingSetupService {
       public:
        virtual ~IStreamingSetupService() = default;

        /**
         * @brief Stream a source.
         * If success will populate the result with:
         *
         * WEBRTC
         * - clientId: the id of the client, used to send the answer.
         * - offer: the json SDP offer
         *
         * MJPEG Stream / JPEG Cache Busting
         * - stream_url: the url to the stream
         *
         * @param uri
         * @param result preparation result
         * @return true success
         */
        virtual bool PrepareStream(
            const std::string& uri,
            std::unordered_map<std::string, std::string>& result) = 0;

        /**
         * @brief Same as above but with uri "observer".
         *
         * @param client
         * @return true
         * @return false
         */
        virtual bool PrepareStreamObserver(
            std::unordered_map<std::string, std::string>& result) = 0;

        /**
         * @brief Called when observer was stopped.
         * This will only stop emitting frames from the observer source.
         */
        virtual void OnObserverStopped() = 0;
    };
}  // namespace Web::Streaming