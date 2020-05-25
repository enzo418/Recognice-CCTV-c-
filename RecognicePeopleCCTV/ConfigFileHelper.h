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
            } else if (id == "type") {
                Utils::toLowerCase(value);
                if (value == "active" || value == "activated" || value == "enabled") {
                    config.type = CAMERA_ACTIVE;
                } else if (value == "sentry") {
                    config.type = CAMERA_SENTRY;
                } else if (value == "disabled") {
                    config.type = CAMERA_DISABLED;
                }
            } else if (id == "hitthreshold") {
                std::replace(value.begin(), value.end(), ',', '.');
                float val = std::stof(value);
                config.hitThreshold = val;
            } else if (id == "changethreshold") {
                int val = std::stoi(value);
                config.changeThreshold = val;
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
            } else if (id == "telegram_bot_api"  || id == "telegram_api"
                       || id == "telegrambotapi" || id == "telegramapi") {
                config.telegramConfig.apiKey = value;
            } else if (id == "telegram_chat_id"  || id == "telegram_bot_chat_id"
                       || id == "telegramchatid" || id == "telegrambotchatid") {
                config.telegramConfig.chatId = value;
            } else if (id == "showpreviewcameras" || id == "showpreviewofcameras") {    
                Utils::toLowerCase(value);
                config.showPreview = value == "no" ? false : true;
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
                                configs.push_back(config);
                            }
                        }
                    }
                }
            }

            _file.close();
        }
};

