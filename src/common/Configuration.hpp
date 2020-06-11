#pragma once
#include <iostream>
#include <vector>
#include "types.hpp"
#include "utils.hpp"
#include "ConfigurationFile.hpp"

namespace Config
{
    void ModifyCamera(std::vector<CameraConfig>& configs);

    void AddNewCamera(std::vector<CameraConfig>& configs);

    void CameraConfiguration(std::vector<CameraConfig>& configs);

    void ProgramConfiguration(ProgramConfig& config);

    void StartConfiguration(std::vector<CameraConfig>& configs, ProgramConfig& programConfig);
}