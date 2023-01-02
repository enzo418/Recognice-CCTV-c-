#include "ServerConfigurationProvider.hpp"

namespace Web {

    std::mutex ServerConfigurationProvider::mutex;
    ServerConfiguration ServerConfigurationProvider::configuration;
    Web::DAL::IServerConfigurationPersistance*
        ServerConfigurationProvider::persistance;

    void ServerConfigurationProvider::Initialize(
        Web::DAL::IServerConfigurationPersistance* pPersistance) {
        persistance = pPersistance;
        configuration = persistance->Get();
    }

    const ServerConfiguration& ServerConfigurationProvider::Get() {
        Guard lock(mutex);
        return configuration;
    }

    void ServerConfigurationProvider::Update(const nlohmann::json& fields) {
        Guard lock(mutex);
        persistance->Update(configuration, fields);
    }

}  // namespace Web