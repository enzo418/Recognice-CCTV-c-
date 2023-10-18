#pragma once

#include <unordered_map>

#include "Server/ServerConfigurationProvider.hpp"
#include "Streaming/PeerStreamingCapabilities.hpp"
#include "Streaming/http/JPEGCacheBustingStreaming.hpp"
#include "Streaming/http/MJPEGStreaming.hpp"
#include "observer/Log/log.hpp"
#include "streaming_include.hpp"
#include "uWebSockets/HttpResponse.h"
#include "uWebSockets/WebSocket.h"

namespace Web::Streaming {
    constexpr const char* StreamTypeJPEGCacheBusting = "jpeg_cache_busting";
    constexpr const char* StreamTypeWebRTC = "webrtc";
    constexpr const char* StreamTypeMJPEG = "mjpeg";

    template <bool SSL>
    class StreamingServiceSelector final {
       private:
        typedef typename uWS::HttpResponse<SSL> Client;

       public:
        StreamingServiceSelector(uWS::App* app,
                                 RecognizeContext* pRecognizeCtx) {
            // JPEG Cache Busting Streaming is always available.
            services[StreamTypeJPEGCacheBusting] =
                std::make_unique<Http::JPEGCacheBustingStreaming<SSL>>(
                    app, pRecognizeCtx);

            // MJPEG Streaming just streams over HTTP so it will always be
            // available to be used in this context.
            services[StreamTypeMJPEG] =
                std::make_unique<Http::MJPEGStreaming<SSL>>(app, pRecognizeCtx);

#if WEB_WITH_WEBRTC
            services[StreamTypeWebRTC] =
                std::make_unique<WebRTC::WebRTCStreamingService<SSL>>(
                    app, pRecognizeCtx);
#endif
        }

        /**
         * @brief Prepare a stream to a client.
         * This will select the best streaming service based on the client
         * capabilities and the server capabilities.
         *
         * @param uri url, uri, file path or reachable stream.
         * @param client http client
         * @param peerCapabilities client capabilities
         * @param result preparation result - will be forwarded to the client so
         * it can connect to the stream.
         * @return true if success
         * @return false
         */
        bool PrepareStream(
            const std::string& uri, PeerStreamingCapabilities peerCapabilities,
            std::unordered_map<std::string, std::string>& result);

        /**
         * @brief Prepare a stream observer.
         * This will select the best streaming service based on the client
         * capabilities and the server capabilities.
         *
         * @param client http client
         * @param peerCapabilities client capabilities
         * @param result preparation result - will be forwarded to the client so
         * it can connect to the stream.
         * @return true if success
         * @return false
         */
        bool PrepareStreamObserver(
            PeerStreamingCapabilities peerCapabilities,
            std::unordered_map<std::string, std::string>& result);

        bool StartMJPEGStreamToClient(const std::string& stream_id,
                                      Client* client);

        /**
         * @brief Called when observer was stopped.
         */
        void OnObserverStopped();

       private:  // services
        std::unordered_map<const char*, std::unique_ptr<IStreamingSetupService>>
            services;  // Poor man's DI

       private:
        const char* GetServiceNameFromCapabilities(
            PeerStreamingCapabilities peerCapabilities,
            PeerStreamingCapabilities serverCapabilities) {
            if (serverCapabilities.supportsWebRTC &&
                peerCapabilities.supportsWebRTC) {
                return StreamTypeWebRTC;
            } else if (serverCapabilities.supportsJpgCacheBusting &&
                       peerCapabilities.supportsJpgCacheBusting) {
                return StreamTypeJPEGCacheBusting;
            } else if (serverCapabilities.supportsMJPEGStream &&
                       peerCapabilities.supportsMJPEGStream) {
                return StreamTypeMJPEG;
            } else {
                return nullptr;
            }
        }
    };

    template <bool SSL>
    bool StreamingServiceSelector<SSL>::PrepareStream(
        const std::string& uri, PeerStreamingCapabilities peerCapabilities,
        std::unordered_map<std::string, std::string>& result) {
        PeerStreamingCapabilities serverStreamingCapabilities =
            ServerConfigurationProvider::Get().serverStreamingCapabilities;

        const char* serviceName = GetServiceNameFromCapabilities(
            peerCapabilities, serverStreamingCapabilities);

        if (serviceName != nullptr) {
            result["type"] = serviceName;
            OBSERVER_ASSERT(services.contains(serviceName),
                            "Streaming service not found");
            if (services.contains(serviceName)) {
                return services[serviceName]->PrepareStream(uri, result);
            } else {
                OBSERVER_ERROR(
                    "Streaming service was but was not registered: {}",
                    serviceName);
                return false;
            }
        }

        OBSERVER_ERROR("No compatible streaming service found");
        return false;
    }

    template <bool SSL>
    bool StreamingServiceSelector<SSL>::PrepareStreamObserver(
        PeerStreamingCapabilities peerCapabilities,
        std::unordered_map<std::string, std::string>& result) {
        PeerStreamingCapabilities serverStreamingCapabilities =
            ServerConfigurationProvider::Get().serverStreamingCapabilities;

        const char* serviceName = GetServiceNameFromCapabilities(
            peerCapabilities, serverStreamingCapabilities);

        if (serviceName != nullptr) {
            result["type"] = serviceName;
            OBSERVER_ASSERT(services.contains(serviceName),
                            "Streaming service not found");
            if (services.contains(serviceName)) {
                return services[serviceName]->PrepareStreamObserver(result);
            } else {
                OBSERVER_ERROR(
                    "Streaming service was but was not registered: {}",
                    serviceName);
                return false;
            }
        }

        OBSERVER_ERROR("No compatible streaming service found");
        return false;
    }

    template <bool SSL>
    void StreamingServiceSelector<SSL>::OnObserverStopped() {
        for (auto& [name, service] : services) {
            service->OnObserverStopped();
        }
    }
}  // namespace Web::Streaming