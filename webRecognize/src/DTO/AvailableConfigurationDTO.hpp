#pragma once

#include <string>

struct AvailableConfigurationDTO {
    std::string name;
    std::string hash;  // an id, hash that defines them e.g. their path.
};