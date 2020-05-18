#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "types.h"
#include "utils.h"

#include <Windows.h>
#include <shlwapi.h>

class ConfigFileHelper {
    private:
        std::string _fileName = "config.ini";
        std::ifstream _file;

        /// <summary> Saves the given id and value into the corresponding member of the camera config </summary>
        void SaveIdVal(CameraConfig& config, std::string id, std::string  value) {
            Utils::toLowerCase(id);

            if (id == "cameraname" || id == "name") {
                config.cameraName = value;
            } else if (id == "order") {
                int val = std::stoi(value);
                config.order = val;
            } else if (id == "url") {
                config.url = value;
            } else if (id == "rotation") {
                int val = std::stoi(value);
                config.rotation = val;
            } else if (id == "sensibility") {
                Utils::DecodeSensibility(config.sensibilityList, value);
            } else if (id == "hitthreshold") {
                std::replace(value.begin(), value.end(), ',', '.');
                float val = std::stof(value);
                config.hitThreshold = val;
            } else if (id == "roi") {
                // convert [(x1,y1), (x2,y2)] to roi struct
                config.roi = Utils::GetROI(value);
            }
        }

        /// <summary> Saves the given id and value into the corresponding member of the program config </summary>
        void SaveIdVal(ProgramConfig& config, std::string id, std::string  value) {
            Utils::toLowerCase(id);

            if (id == "msbetweenframe" || id == "millisbetweenframe") {
                int val = std::stoi(value);
                config.msBetweenFrame = (ushort)val;
            } else if (id == "secondsbetweenimage") {
                std::replace(value.begin(), value.end(), ',', '.');
                float val = std::stof(value);
                config.secondsBetweenImage = val;
            }
        }

        template<typename T>
        T ReadNextConfig(std::ifstream& file, T& config) {
            std::string line;

            while (!Utils::nextLineIsHeader(file) && std::getline(file, line)) {
                std::string id;
                std::string val;

                Utils::trim(line);
                char* str;
                ushort found = 0;

                int indx = strcspn(line.c_str(), "=");

                id = line.substr(0, indx);
                val = line.substr(indx+1, line.size() - 1);

                if (id.size() > 0 && val.size() > 0) {
                    SaveIdVal(config, id, val);
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

            std::cout << _fileName << std::endl;
            std::cout << filename << std::endl;

            if (Utils::FileExist(filename)) {
                _file.open(filename);
            }
        };

        ConfigFileHelper(std::string filePath) : _fileName(filePath) {
            ConfigFileHelper();
        };

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
                            configs.push_back(ReadNextConfig(_file, config));
                        }
                    }
                }
            }

            _file.close();
        }
};

