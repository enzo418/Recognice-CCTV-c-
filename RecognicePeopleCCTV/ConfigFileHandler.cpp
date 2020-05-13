#include <iostream>
#include <fstream>
#include <vector>
#include "types.h"

void ValidateIdentifierValue(CameraConfig &config, std::string id, std::string  value) {
    if (id == "cameraName") {
        config.cameraName = value;
    } // ...
}

CameraConfig ReadCameraConfig(std::fstream& file) {
    CameraConfig config;
    
    char c;

                        // id = value

    std::string id;     // identifier
    std::string value;  // value

    bool isId = true;

    file.get(c);
        
    while (!file.eof()) {
        if (c != '=' && c >= 'A' && isId) {
            id += c;
        } else if (c > 32 && (!isId || c == '=')) {
            isId = false;
            value += c;
        } else if (c == '\n') {
            ValidateIdentifierValue(config, id, value);
            
            id = "";
            value = "";
        }

        file.get(c);
    }
}

void ReadFile(std::fstream file, ProgramConfig& programConfig, CameraConfig* configs, ushort configsSize) {
    char* line;
    std::streamsize lineSize;
    
    std::vector<CameraConfig> _configs;
    
    while (file.getline(line, lineSize)) {
        if (line[0] == '[') {
            strtok(line, "]");
            if (line != NULL) {
                line[0] = '\0';
                if (line == "PROGRAM") {
                    // config of program
                } else if (line == "CAMERA") {
                    // create config of camera
                    _configs.push_back(ReadCameraConfig(file));
                }
            }
        }
    }

    file.close();
}