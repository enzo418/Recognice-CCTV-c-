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
         */
        nldb::json Get(const std::string& id);

        nldb::json GetCamera(const std::string& id);

        /**
         * @brief Insert a new configuration.
         *
         * @param config
         * @return std::string id of the new configuration
         */
        std::string InsertConfiguration(const nldb::json& config);

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