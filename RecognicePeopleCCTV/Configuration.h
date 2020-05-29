#pragma once
#include <iostream>
#include <vector>
#include "types.h"
#include "utils.h"

namespace Config
{
    void SaveIdVal(CameraConfig& config, std::string id, std::string  value);

    void SaveIdVal(ProgramConfig& config, std::string id, std::string  value);

    void ModifyCamera(std::vector<CameraConfig>& configs);

    void AddNewCamera(std::vector<CameraConfig>& configs);

    void CameraConfiguration(std::vector<CameraConfig>& configs);

    void ProgramConfiguration(ProgramConfig& config);

    void StartConfiguration(std::vector<CameraConfig>& configs, ProgramConfig& programConfig);
}