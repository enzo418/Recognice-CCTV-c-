#pragma once

#include "DTO/AvailableConfigurationDTO.hpp"
#include "DTO/AvailableConfigurationsDTO.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann {

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AvailableConfigurationDTO, name, hash);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AvailableConfigurationsDTO, names);

}  // namespace nlohmann
