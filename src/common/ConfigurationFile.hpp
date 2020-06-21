#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <sstream>
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
        class ConfigFileHelper {
        private:
            //const char* _fileName = "./build/config.ini";
            const char* _fileName = "./config.ini";
            std::fstream _file;
            char openMode = '\0';

            template<typename T>
            T ReadNextConfig(std::fstream& file, T& config);

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
                if (_file.is_open()){ 
                    if (openMode != 'r')
                        _file.close();
                    else return;
                }

                char filename[MAX_PATH] = {};
                strcpy_s(filename, GetFilePath(_fileName));

                openMode = 'r';

                if (Utils::FileExist(filename)) {
                    _file.open(filename);
                }
            }

            void OpenFileWrite() {
                if (_file.is_open()){
                    if(openMode != 'w')
                        _file.close();
                    else return;
                }

                char filename[MAX_PATH] = {};
                strcpy_s(filename, GetFilePath(_fileName));

                openMode = 'w';
                
                _file.open(filename, std::ios::app);                
            }

        public:
            ConfigFileHelper() {
                OpenFileRead();
            };

            ConfigFileHelper(const char* filePath) : _fileName(filePath) {
                OpenFileRead();
            };

            /// <summary> Reads the config file then builds and return the configurations</summary>
            void ReadFile(ProgramConfig& programConfig, std::vector<CameraConfig>& configs);

            void WriteFile();

            void WriteInFile(std::vector<CameraConfig>& configs);

            inline void WriteLineInFile(const char* line);

            void WriteInFile(CameraConfig& cfg);
        };        
    };
};

