#pragma once

#include <string>

#include "Server/ServerConfiguration.hpp"
#include "nlohmann/json.hpp"

namespace Web::DAL {

    class IServerConfigurationPersistance {
       public:
        virtual ServerConfiguration Get() = 0;

        /**
         * @brief Updates the configuration, stores the changes and modifies the
         * first parameter with the new values.
         *
         * @param toUpdate
         * @param fields
         */
        virtual void Update(ServerConfiguration& toUpdate,
                            const nlohmann::json& fields) = 0;
    };
}  // namespace Web::DAL
