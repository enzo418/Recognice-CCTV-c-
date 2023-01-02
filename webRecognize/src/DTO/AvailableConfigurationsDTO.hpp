#pragma once

#include <string>
#include <vector>

#include "./AvailableConfigurationDTO.hpp"

struct AvailableConfigurationsDTO {
    std::vector<AvailableConfigurationDTO> names;
};