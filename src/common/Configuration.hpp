#pragma once
#include <iostream>
#include <vector>
#include "types.hpp"
#include "utils.hpp"
#include "ConfigurationFile.hpp"

namespace Config
{
    void ModifyCamera(std::vector<CameraConfig>& configs, Config::File::ConfigFileHelper& fh);

    void AddNewCamera(std::vector<CameraConfig>& configs, Config::File::ConfigFileHelper& fh);

    void SetAreaDelimitersCamera(std::vector<CameraConfig>& configs, Config::File::ConfigFileHelper& fh);

    void CameraConfiguration(std::vector<CameraConfig>& configs, Config::File::ConfigFileHelper& fh);

    void ProgramConfiguration(ProgramConfig& config, Config::File::ConfigFileHelper& fh);

    void StartConfiguration(std::vector<CameraConfig>& configs, ProgramConfig& programConfig, Config::File::ConfigFileHelper& fh);
}