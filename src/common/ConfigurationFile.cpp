#include "ConfigurationFile.hpp"

/// <summary> Saves the given id and value into the corresponding member of the camera config </summary>
void Config::SaveIdVal(CameraConfig& config, std::string id, std::string value) {
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
	} else if (id == "secondswaitentryexit") {
		int val = std::stoi(value);
		config.secondsWaitEntryExit = val;
	} else if (id == "areasdelimiters" || id == "areadelimiters"){
		config.areasDelimiters = Utils::StringToAreaDelimiters(value.c_str(), config.roi);
	}
}

/// <summary> Saves the given id and value into the corresponding member of the program config </summary>
void Config::SaveIdVal(ProgramConfig& config, std::string id, std::string value) {
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
	} else if (id == "sendimagewhendetectchange" || id == "sendimageafterdetectigchange"){
		config.sendImageWhenDetectChange = value == "no" ? false : true;
	} else if (id == "usetelegrambot" || id == "activatetelegrambot") {
		Utils::toLowerCase(value);
		config.telegramConfig.useTelegramBot = value == "no" ? false : true;
	}
}

void Config::File::AppendCameraConfig(CameraConfig& cfg) {
	ConfigFileHelper fh;
	fh.WriteInFile(cfg);
}

template<typename T>
T Config::File::ConfigFileHelper::ReadNextConfig(std::fstream& file, T& config) {
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

void Config::File::ConfigFileHelper::ReadFile(ProgramConfig& programConfig, std::vector<CameraConfig>& configs) {
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

inline void Config::File::ConfigFileHelper::WriteLineInFile(const char* line){
	_file.write(line, sizeof(char) * strlen(line));
}

void Config::File::ConfigFileHelper::WriteFile() {
	OpenFileWrite();
}

// Dumps all the camera configs to the file
void Config::File::ConfigFileHelper::WriteInFile(std::vector<CameraConfig>& configs) {
	OpenFileWrite();

	for(auto & config: configs)
		ConfigFileHelper::WriteInFile(config);
}

// Writes a camera configuration to the file
void Config::File::ConfigFileHelper::WriteInFile(CameraConfig& cfg){	
	OpenFileWrite();

	std::string tmp;

	// Write header
	ConfigFileHelper::WriteLineInFile("\n[CAMERA]");

	tmp = "cameraName=" + cfg.cameraName;                
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	tmp = "order=" + cfg.order;
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	tmp = "url=" + cfg.url;
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	tmp = "rotation=" + cfg.rotation;
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	tmp = "type=" + cfg.type;
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	std::ostringstream strs;
	strs << cfg.hitThreshold;
	tmp = "hitThreshold=" + strs.str();                
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	tmp = "changeThreshold=" + cfg.changeThreshold;
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	tmp = "roi=" + Utils::RoiToString(cfg.roi);
	ConfigFileHelper::WriteLineInFile(tmp.c_str());

	strs.clear();
	strs << cfg.noiseThreshold;
	tmp = "thresholdNoise=" + strs.str();  
	ConfigFileHelper::WriteLineInFile(tmp.c_str());
}