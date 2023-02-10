#pragma once

#include <stdexcept>
#include <string>

#include "nldb/Collection.hpp"
#include "nldb/SQL3Implementation.hpp"

namespace Web::DAL {

    class IConfigurationDAO {
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
        virtual nldb::json GetConfiguration(const std::string& id) = 0;

        /**
         * @brief Get the Camera object
         *
         * @param id
         * @return nldb::json
         * @throws std::runtime_error on camera not found
         */
        virtual nldb::json GetCamera(const std::string& id) = 0;

        /**
         * @brief Adds a new camera to a configuration
         *
         * @param configurationID target configuration
         * @return std::string
         */
        virtual std::string AddCameraToConfiguration(
            const std::string& configurationID,
            const nldb::json& cameraConfiguration) = 0;

        /**
         * @brief Deletes a camera from a configuration
         *
         * @param configurationID
         * @param cameraID
         * @throws std::runtime_error on configuration not found
         */
        virtual void DeleteCameraFromConfiguration(
            const std::string& configurationID,
            const std::string& cameraID) = 0;

        /**
         * @brief Search for a camera in a configuration based on its name
         *
         * @param configuration_id
         * @param cameraName
         * @return nldb::json
         * @throw std::runtime_error if configuration wasn't found
         * @throw std::runtime_error if camera wasn't found
         */
        virtual nldb::json FindCamera(const std::string& configuration_id,
                                      const std::string& cameraName) = 0;

        /**
         * @brief Insert a new configuration.
         *
         * @param config
         * @return std::string id of the new configuration
         */
        virtual std::string InsertConfiguration(const nldb::json& config) = 0;

        virtual void DeleteConfiguration(const std::string& id) = 0;

        virtual void UpdateConfiguration(const std::string& id,
                                         const nldb::json& data) = 0;

        virtual void UpdateCamera(const std::string& id,
                                  const nldb::json& data) = 0;

        // Get all the ids and names of the configurations stored.
        virtual nldb::json GetAllNamesAndId() = 0;
    };
}  // namespace Web::DAL