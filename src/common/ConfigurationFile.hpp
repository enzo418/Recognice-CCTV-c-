#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#ifdef WINDOWS
#include <Windows.h>
#include <shlwapi.h>
#else
#define strcpy_s(x,y) strcpy(x,y)
#define MAX_PATH 260
#endif

#include "types.hpp"
#include "utils.hpp"

namespace Config
{
    void SaveIdVal(CameraConfig& config, std::string id, std::string value);
    void SaveIdVal(ProgramConfig& config, std::string id, std::string value);

    namespace File
    {
        void AppendCameraConfig(CameraConfig& cfg);

        class ConfigFileHelper {
        private:
            //const char* _fileName = "./build/config.ini";
            const char* _fileName = "./config.ini";
            std::fstream _file;

            template<typename T>
            T ReadNextConfig(std::fstream& file, T& config) {
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
                            Config::SaveIdVal(config, id, val);
                        }
                    }
                }

                return config;
            }

            const char* GetFilePath(const char* fileName) {
				#ifdef WINDOWS
                char path[MAX_PATH] = {};
                GetModuleFileNameA(NULL, path, MAX_PATH);
                PathRemoveFileSpecA(path);
                PathCombineA(path, path, fileName);
                return path;
				#else
				return fileName;
				#endif
            }

            void OpenFileRead() {
                if (_file.is_open()) _file.close();

                char filename[MAX_PATH] = {};
                strcpy_s(filename, GetFilePath(_fileName));

                if (Utils::FileExist(filename)) {
                    _file.open(filename);
                }
            }

            void OpenFileWrite() {
                if (_file.is_open()) _file.close();

                char filename[MAX_PATH] = {};
                strcpy_s(filename, GetFilePath(_fileName));

                if (Utils::FileExist(filename)) {
                    _file.open(filename, std::ios::app);
                }
            }

        public:
            ConfigFileHelper() {
                OpenFileRead();
            };

            ConfigFileHelper(const char* filePath) : _fileName(filePath) {
                OpenFileRead();
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

            void WriteFile() {
                OpenFileWrite();
            }

            void WriteInFile(std::vector<CameraConfig>& configs) {
                OpenFileWrite();
            }

            void WriteInFile(CameraConfig& cfg) {
                OpenFileWrite();

                // Write header
                const char header[] = "\n[CAMERA]";
                _file.write(header, sizeof(char) * strlen(header));


            }
        };        
    };
};

