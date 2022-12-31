#pragma once

#include <mutex>

#include "DAL/IServerConfigurationPersistance.hpp"
#include "ServerConfiguration.hpp"

namespace Web {

    class ServerConfigurationProvider {
       public:
        /**
         * @brief Initializes the provider.
         * Must be exactly one time.
         *
         */
        static void Initialize(
            Web::DAL::IServerConfigurationPersistance* persistance);

        /**
         * @brief Get the current server configuration
         *
         * @return ServerConfiguration const&
         */
        static const ServerConfiguration& Get();

        /**
         * @brief Update the current configuration fields
         *
         * @param fields
         */
        static void Update(const nlohmann::json& fields);

       private:
        static std::mutex mutex;
        static ServerConfiguration configuration;

        static Web::DAL::IServerConfigurationPersistance* persistance;

        ServerConfigurationProvider() = default;

       private:
        typedef std::lock_guard<std::mutex> Guard;
    };
}  // namespace Web