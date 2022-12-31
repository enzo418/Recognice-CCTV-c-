#pragma once

#include <fstream>

#include "../IServerConfigurationPersistance.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "nlohmann/json.hpp"

namespace Web::DAL {
    /**
     * @brief Writes/read the configuration into a json file.
     * There is no lock mechanism in this class.
     */
    class ServerConfigurationPersistanceFile final
        : public IServerConfigurationPersistance {
       public:
        ServerConfigurationPersistanceFile(const std::string& pFilename)
            : fileName(pFilename) {}

        ServerConfiguration Get() override {
            std::ifstream f(fileName);
            if (f.peek() != std::ifstream::traits_type::eof()) {
                nlohmann::json config = nlohmann::json::parse(f);
                return config;
            } else {
                return {};  // use default values
            }
        }

        void Update(ServerConfiguration& toUpdate,
                    const nlohmann::json& fields) override {
            nlohmann::json config = toUpdate;

            // apply changes
            config.merge_patch(fields);

            // save into file
            std::ofstream f(fileName, std::ios_base::trunc);
            f << config;

            // update reference value
            toUpdate = config;
        }

       private:
        std::string fileName;
    };
}  // namespace Web::DAL