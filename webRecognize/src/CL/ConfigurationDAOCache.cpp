#include "ConfigurationDAOCache.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace Web::CL {

    ConfigurationDAOCache::ConfigurationDAOCache(
        std::unique_ptr<IConfigurationDAO>&& pRepository)
        : repository(std::move(pRepository)) {}

    nldb::json ConfigurationDAOCache::GetConfiguration(const std::string& id) {
        if (configurationCache.contains(id)) {
            return configurationCache.get(id);
        } else {
            auto cfg = repository->GetConfiguration(id);
            configurationCache.insert(id, cfg);
            return cfg;
        }
    }

    nldb::json ConfigurationDAOCache::GetCamera(const std::string& id) {
        if (cameraCache.contains(id)) {
            return cameraCache.get(id);
        } else {
            auto cfg = repository->GetCamera(id);
            cameraCache.insert(id, cfg);
            return cfg;
        }
    }

    std::string ConfigurationDAOCache::AddCameraToConfiguration(
        const std::string& configurationID,
        const nldb::json& cameraConfiguration) {
        return repository->AddCameraToConfiguration(configurationID,
                                                    cameraConfiguration);
    }

    void ConfigurationDAOCache::DeleteCameraFromConfiguration(
        const std::string& configurationID, const std::string& cameraID) {
        repository->DeleteCameraFromConfiguration(configurationID, cameraID);
    }

    nldb::json ConfigurationDAOCache::FindCamera(
        const std::string& configuration_id, const std::string& cameraName) {
        return repository->FindCamera(configuration_id, cameraName);
    }

    std::string ConfigurationDAOCache::InsertConfiguration(
        const nldb::json& pConfig) {
        return repository->InsertConfiguration(pConfig);
    }

    void ConfigurationDAOCache::DeleteConfiguration(const std::string& id) {
        repository->DeleteConfiguration(id);

        if (configurationCache.contains(id)) {
            configurationCache.remove(id);
        }
    }

    void ConfigurationDAOCache::UpdateConfiguration(const std::string& id,
                                                    const nldb::json& data) {
        repository->UpdateConfiguration(id, data);

        if (configurationCache.contains(id)) {
            configurationCache.remove(id);
        }
    }

    void ConfigurationDAOCache::UpdateCamera(const std::string& id,
                                             const nldb::json& data) {
        repository->UpdateCamera(id, data);

        if (cameraCache.contains(id)) {
            cameraCache.remove(id);
        }
    }

    nldb::json ConfigurationDAOCache::GetAllNamesAndId() {
        return repository->GetAllNamesAndId();
    }
}  // namespace Web::CL