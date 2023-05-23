#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "DAL/IConfigurationDAO.hpp"
#include "nldb/Collection.hpp"
#include "nldb/SQL3Implementation.hpp"

namespace Web::CL {
    class ConfigurationDAOCache final : public DAL::IConfigurationDAO {
       public:
        ConfigurationDAOCache(std::unique_ptr<IConfigurationDAO>&& repository);

       public:
        /**
         * @brief Get a configuration (WEB).
         * This configuration might contain cameras with name, url, etc.
         * Can be parsed into a Configuration struct with the help of
         * Observer::ConfigurationParser.
         *
         * @param id
         * @return nldb::json
         * @throws std::runtime_error on config not found
         */
        nldb::json GetConfiguration(const std::string& id) override;

        /**
         * @brief Get the Camera object
         *
         * @param id
         * @return nldb::json
         * @throws std::runtime_error on camera not found
         */
        nldb::json GetCamera(const std::string& id) override;

        /**
         * @brief Adds a new camera to a configuration
         *
         * @param configurationID target configuration
         * @return std::string
         */
        std::string AddCameraToConfiguration(
            const std::string& configurationID,
            const nldb::json& cameraConfiguration) override;

        /**
         * @brief Deletes a camera from a configuration
         *
         * @param configurationID
         * @param cameraID
         * @throws std::runtime_error on configuration not found
         */
        void DeleteCameraFromConfiguration(
            const std::string& configurationID,
            const std::string& cameraID) override;

        /**
         * @brief Search for a camera in a configuration based on its name
         *
         * @param configuration_id
         * @param cameraName
         * @return nldb::json
         * @throw std::runtime_error if configuration wasn't found
         * @throw std::runtime_error if camera wasn't found
         */
        nldb::json FindCamera(const std::string& configuration_id,
                              const std::string& cameraName) override;

        /**
         * @brief Insert a new configuration.
         *
         * @param config
         * @return std::string id of the new configuration
         */
        std::string InsertConfiguration(const nldb::json& config) override;

        void DeleteConfiguration(const std::string& id) override;

        void UpdateConfiguration(const std::string& id,
                                 const nldb::json& data) override;

        void UpdateCamera(const std::string& id,
                          const nldb::json& data) override;

        // Get all the ids and names of the configurations stored.
        nldb::json GetAllNamesAndId() override;

       private:
        std::unique_ptr<IConfigurationDAO> repository;

        typedef lru11::Cache<std::string, nldb::json> ConfigCache;

        ConfigCache configurationCache;
        lru11::Cache<std::string, nldb::json> cameraCache;
    };
}  // namespace Web::CL