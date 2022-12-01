#pragma once

#include <stdexcept>
#include <string>

#include "Utils/JsonLibrariesConversion.hpp"
#include "nldb/SQL3Implementation.hpp"
#include "yaml-cpp/node/node.h"

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
         * @return YAML::Node
         */
        YAML::Node Get(const std::string& id);

        /**
         * @brief Insert a new configuration.
         *
         * @param config
         * @return std::string id of the new configuration
         */
        std::string InsertConfiguration(const nldb::json& config);

        void UpdateConfiguration(const std::string& id, const nldb::json& data);

        void UpdateCamera(const std::string& id, const nldb::json& data);

       private:
        void AddCamerasToConfiguration(nldb::json& cfg);

        nldb::DBSL3* db;
        nldb::Query<nldb::DBSL3> query;
    };
}  // namespace Web::DAL