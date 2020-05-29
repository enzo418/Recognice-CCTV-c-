#include "Configuration.h"

namespace Config
{
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
        } else if (id == "thresholdnoise" || id == "noisethreshold") {
            std::replace(value.begin(), value.end(), ',', '.');
            double val = std::stof(value);
            config.noiseThreshold = val;
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
        } else if (id == "secondsbetweenmessage") {
            int val = std::stoi(value);
            config.secondsBetweenMessage = val;
        } else if (id == "outputresolution") {
            size_t indx = value.find(",");
            if (indx > 0) {
                try {
                    config.outputResolution.width = std::stoi(value.substr(0, indx));
                    config.outputResolution.height = std::stoi(value.substr(indx + 1, value.size() - 1));
                } catch (std::invalid_argument e) {
                    std::cout << "Invalid input at option \"outputResolution\" in config.ini header PROGRAM. Expected format: width,height.\n";                    
                    std::getchar();
                    exit(-1);
                }
            }
        } else if (id == "telegram_bot_api" || id == "telegram_api"
                   || id == "telegrambotapi" || id == "telegramapi") {
            config.telegramConfig.apiKey = value;
        } else if (id == "telegram_chat_id" || id == "telegram_bot_chat_id"
                   || id == "telegramchatid" || id == "telegrambotchatid") {
            config.telegramConfig.chatId = value;
        } else if (id == "showpreviewcameras" || id == "showpreviewofcameras") {
            Utils::toLowerCase(value);
            config.showPreview = value == "no" ? false : true;
        } else if (id == "showareacamerasees" || id == "showareacamera") {
            Utils::toLowerCase(value);
            config.showAreaCameraSees = value == "no" ? false : true;
        } else if (id == "showprocessedframes" || id == "showprocessedimages") {
            Utils::toLowerCase(value);
            config.showProcessedFrames = value == "no" ? false : true;
        }
    }

    void LoadConfigCamera(CameraConfig& src, CameraConfig& dst, bool isModification = false) {
        std::string input;        

        while (1) {
            std::cout << "Enter the camera name: ";
            if (isModification) std::cout << "[" << src.cameraName << "]";
            std::getline(std::cin, dst.cameraName);

            std::cout << "Enter the camera url: ";
            if (isModification) std::cout << "[" << src.url << "]";
            std::getline(std::cin, dst.url);

            std::cout << "Enter the camera roi";
            if (isModification) std::cout << "[" << Utils::RoiToString(src.roi) << "]";
            else std::cout << ", format [(x1,y1),(x2,y2)]: ";
            std::getline(std::cin, input);
            if (input.size() > 0)
                dst.roi = Utils::GetROI(input);

            std::cout << "Enter camera order: ";
            if (isModification) std::cout << "[" << src.order << "]";
            std::getline(std::cin, input);
            if (input.size() > 0)
                dst.order = std::stoi(input);

            std::cout << "Enter rotation (deg): ";
            if (isModification) std::cout << "[" << src.rotation << "]";
            std::getline(std::cin, input);
            if (input.size() > 0)
                dst.rotation = std::stoi(input);

            std::cout << "Enter camera change threshold: ";
            if (isModification) std::cout << "[" << src.changeThreshold << "]";
            std::getline(std::cin, input);
            if (input.size() > 0)
                dst.changeThreshold = std::stoi(input);

            std::cout << "Enter the camera type Sentry, Active, Disabled: ";
            if (isModification) std::cout << "[" << Utils::CamTypeToString(src.type) << "]";
            std::getline(std::cin, input);
            Utils::toLowerCase(input);
            if (input == "sentry") {
                dst.type = CAMERA_SENTRY;
            } else if (input == "disabled") {
                dst.type = CAMERA_DISABLED;
            } else {
                dst.type = CAMERA_ACTIVE;
            }

            std::cout << "Enter the hit threshold (0-1): ";
            if (isModification) std::cout << "[" << src.hitThreshold << "]";
            std::getline(std::cin, input);
            if (input.size() > 0)
                SaveIdVal(dst, "hitthreshold", input);

            std::cout << "Enter the noise threshold: ";
            if (isModification) std::cout << "[" << src.noiseThreshold << "]";
            std::getline(std::cin, input);
            SaveIdVal(dst, "noisethreshold", input);

            std::cout << "Exit and save changes? (yes/no):";
            std::getline(std::cin, input);
            if (input == "yes" || input == "YES") {
                src = dst;
                return;
            } else {
                std::cout << "Edit again? (yes/no):";
                std::getline(std::cin, input);
                if (input == "no" || input == "NO") return;
            }
        }
    }

    void ModifyCamera(std::vector<CameraConfig>& configs) {
        std::cout << "# Modify camera config\n";
        std::string input;

        size_t size = configs.size();

        for (size_t i = 1; i <= size; i++) {
            std::cout << "\t" << i << ". " << configs[i].cameraName;
            std::cout << "\t  " << "url=" << configs[i].url;
        }

        std::cout << "- Please enter a camera config:";
        std::getline(std::cin, input);
        int indx = std::stoi(input) - 1;

        if (indx > 0 && indx < size) {
            LoadConfigCamera(configs[indx], configs[indx], true);
            // Write config in file
        } else std::cout << "ERROR: Invalid index " << indx << std::endl;
    }

    void AddNewCamera(std::vector<CameraConfig>& configs) {
        CameraConfig config;

        LoadConfigCamera(config, config, false);

        // Write cameraconfig in file

        if (config.type != CAMERA_DISABLED) configs.push_back(config);
    }

    void CameraConfiguration(std::vector<CameraConfig>& configs) {
        char opt = 0;
        while (opt != '1' && opt != '2') {
            std::cout << "\n# Cameras configurations" << std::endl
                << "\t1. Add new camera" << std::endl
                << "\t2. Modify camera" << std::endl
                << "- Chose a option: ";

            opt = std::getchar();
        }

        if (opt == '1') {
            AddNewCamera(configs);
        } else if (opt == '2') {
            ModifyCamera(configs);
        }
    }

    void ProgramConfiguration(ProgramConfig& config) {

    }

    void StartConfiguration(std::vector<CameraConfig>& configs, ProgramConfig& programConfig) {
        char opt = 0;
        while (opt != '1' && opt != '2') {
            std::cout << "# Configuration" << std::endl
                << "\t1. Cameras" << std::endl
                << "\t2. Program" << std::endl
                << "- Chose a option: ";

            opt = std::getchar();
        }

        if (opt == '1') {
            CameraConfiguration(configs);
        } else if (opt == '2') {
            ProgramConfiguration(programConfig);
        }
    }
}