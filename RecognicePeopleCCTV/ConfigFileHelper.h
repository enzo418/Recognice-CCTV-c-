#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <Windows.h>
#include <shlwapi.h>

#include "Configuration.h"

namespace Config
{
    class ConfigFileHelper {
    private:
        std::string _fileName = "config.ini";
        std::ifstream _file;

        template<typename T>
        T ReadNextConfig(std::ifstream& file, T& config) {
            std::string line;

            while (!Utils::nextLineIsHeader(file) && std::getline(file, line)) {
                if (line.size() > 3 && line[0] != '#') {
                    std::string id;
                    std::string val;

                    Utils::trim(line);
                    char* str;
                    ushort found = 0;

                    int indx = strcspn(line.c_str(), "=");

                    id = line.substr(0, indx);
                    val = line.substr(indx + 1, line.size() - 1);

                    if (id.size() > 0 && val.size() > 0) {
                        SaveIdVal(config, id, val);                        
                    }
                }
            }

            return config;
        }

    public:
        ConfigFileHelper() {
            char filename[MAX_PATH] = {};

            GetModuleFileNameA(NULL, filename, MAX_PATH);
            PathRemoveFileSpecA(filename);
            PathCombineA(filename, filename, "config.ini");

            if (Utils::FileExist(filename)) {
                _file.open(filename);
            }
        };

        ConfigFileHelper(std::string filePath) : _fileName(filePath) {
            ConfigFileHelper();
        };

        /// <summary> Reads the config file then builds and return the configurations</summary>
        void ReadFile(ProgramConfig& programConfig, std::vector<CameraConfig>& configs) {
            std::string line;

            while (std::getline(_file, line)) {
                if (line != "") {
                    if (line[0] == '[') {
                        line = line.substr(1, line.size() - 2);
                        Utils::toLowerCase(line);

                        if (line == "program") {
                            ProgramConfig config;
                            programConfig = ReadNextConfig(_file, config);
                        } else if (line == "camera") {
                            CameraConfig config;
                            ReadNextConfig(_file, config);

                            // validate config
                            if (config.type == CAMERA_DISABLED) {
                                std::cout << config.cameraName << " skiped because its type is disabled" << std::endl;
                            } else if (config.url.empty() /* check if is a valid url*/) {
                                std::cout << config.cameraName << " skiped because its url is not valid" << std::endl;
                            } else {
                                if (config.noiseThreshold == 0) {
                                    std::cout << "[Warning] camera option \"noiseThreshold\" is 0, this can cause problems." << std::endl;
                                }

                                configs.push_back(config);
                            }
                        }
                    }
                }
            }

            _file.close();
        }
    };
}

