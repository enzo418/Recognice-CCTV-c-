#pragma once

#include "../../vendor/json_dto/json_dto.hpp"
#include "AvailableConfigurationDTO.hpp"
#include "AvailableConfigurationsDTO.hpp"

namespace json_dto {

    template <typename Json_Io>
    void json_io(Json_Io& io, AvailableConfigurationDTO& msg) {
        io& json_dto::mandatory("name", msg.name) &
            json_dto::mandatory("hash", msg.hash);
    }

    template <typename Json_Io>
    void json_io(Json_Io& io, AvailableConfigurationsDTO& msg) {
        io& json_dto::mandatory("names", msg.names);
    }

} /* namespace json_dto */
