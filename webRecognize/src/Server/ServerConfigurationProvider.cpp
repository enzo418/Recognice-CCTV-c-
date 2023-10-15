#include "ServerConfigurationProvider.hpp"

#include "observer/Log/log.hpp"

namespace Web {

    std::mutex ServerConfigurationProvider::mutex;
    ServerConfiguration ServerConfigurationProvider::configuration;
    Web::DAL::IServerConfigurationPersistance*
        ServerConfigurationProvider::persistance;

    void ServerConfigurationProvider::Initialize(
        Web::DAL::IServerConfigurationPersistance* pPersistance) {
        persistance = pPersistance;
        configuration = persistance->Get();

        AlignConfigurationToBuild();
    }

    const ServerConfiguration& ServerConfigurationProvider::Get() {
        Guard lock(mutex);
        return configuration;
    }

    void ServerConfigurationProvider::Update(const nlohmann::json& fields) {
        Guard lock(mutex);
        persistance->Update(configuration, fields);
    }

    void ServerConfigurationProvider::AlignConfigurationToBuild() {
#if !WEB_WITH_WEBRTC
        configuration.serverStreamingCapabilities.supportsWebRTC = false;

        OBSERVER_INFO(
            "Disabling WebRTC streaming: build without WebRTC support.");
#endif

#if !WEB_WITH_H264_ENCODER
        configuration.serverStreamingCapabilities.supportsH264Stream = false;

        OBSERVER_INFO(
            "Disabling H264 streaming: build without H264 encoder support.");
#endif
    }
}  // namespace Web