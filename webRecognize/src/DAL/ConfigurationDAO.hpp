#pragma once

#include <stdexcept>
#include <string>

#include "nldb/Collection.hpp"
#include "nldb/SQL3Implementation.hpp"

namespace Web::DAL {
    class ConfigurationDAO {
       public:
        ConfigurationDAO(nldb::DBSL3* db);

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
        nldb::json GetConfiguration(const std::string& id);

        /**
         * @brief Get the Camera object
         *
         * @param id
         * @return nldb::json
         * @throws std::runtime_error on camera not found
         */
        nldb::json GetCamera(const std::string& id);

        /**
         * @brief Adds a new camera to a configuration
         *
         * @param configurationID target configuration
         * @return std::string
         */
        std::string AddCameraToConfiguration(
            const std::string& configurationID,
            const nldb::json& cameraConfiguration);

        /**
         * @brief Deletes a camera from a configuration
         *
         * @param configurationID
         * @param cameraID
         * @throws std::runtime_error on configuration not found
         */
        void DeleteCameraFromConfiguration(const std::string& configurationID,
                                           const std::string& cameraID);

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
                              const std::string& cameraName);

        /**
         * @brief Insert a new configuration.
         *
         * @param config
         * @return std::string id of the new configuration
         */
        std::string InsertConfiguration(const nldb::json& config);

        void DeleteConfiguration(const std::string& id);

        void UpdateConfiguration(const std::string& id, const nldb::json& data);

        void UpdateCamera(const std::string& id, const nldb::json& data);

        // Get all the ids and names of the configurations stored.
        nldb::json GetAllNamesAndId();

       private:
        void AddCamerasToConfiguration(nldb::json& cfg);

        nldb::DBSL3* db;
        nldb::Query<nldb::DBSL3> query;
        nldb::Collection colConfiguration;
        nldb::Collection colCamera;
    };
}  // namespace Web::DAL