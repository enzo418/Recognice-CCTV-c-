#pragma once
#include <iostream>
#include <vector>
#include "types.h"
#include "utils.h"
#include "ConfigurationFile.h"

namespace Config
{
    void ModifyCamera(std::vector<CameraConfig>& configs);

    void AddNewCamera(std::vector<CameraConfig>& configs);

    void CameraConfiguration(std::vector<CameraConfig>& configs);

    void ProgramConfiguration(ProgramConfig& config);

    void StartConfiguration(std::vector<CameraConfig>& configs, ProgramConfig& programConfig);
}