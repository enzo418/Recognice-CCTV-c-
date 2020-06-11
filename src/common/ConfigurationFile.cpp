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
	}
}

//namespace Config::File
//{
	void Config::File::AppendCameraConfig(CameraConfig& cfg) {
		ConfigFileHelper fh;
		fh.WriteInFile(cfg);
	}
//};