#include "configuration_file.hpp"

namespace ConfigurationFile {
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

	void OpenFileRead(std::fstream& file, std::string& fn){
		if (!file.is_open()){ 
			char filename[MAX_PATH] = {};
			strcpy_s(filename, GetFilePath(fn.c_str()));
			if (Utils::FileExist(filename)) {
				file.open(filename, std::fstream::in);
			}
		}
	}

	void OpenFileWrite(std::fstream& file, std::string& fn){
		if (!file.is_open()){ 
			char filename[MAX_PATH] = {};
			strcpy_s(filename, GetFilePath(fn.c_str()));
			if (Utils::FileExist(filename)) {
				file.open(filename, std::ofstream::out | std::ofstream::trunc);
			}
		}
	}

	inline void WriteLineInFile(std::fstream& file, const char* line){
		file.write(line, sizeof(char) * strlen(line));
	}

	template<typename T>
	T ReadNextConfiguration(std::fstream& file, T& config) {
		std::string line;

		while (!Utils::nextLineIsHeader(file) && std::getline(file, line)) {
			if (line.size() > 3 && line[0] != '#' && line[0] != ';') {
				std::string id;
				std::string val;

				Utils::trim(line);
				char* str;
				ushort found = 0;

				int indx = strcspn(line.c_str(), "=");

				id = line.substr(0, indx);
				val = line.substr(indx + 1, line.size() - 1);

				if (id.size() > 0 && val.size() > 0) {
					ConfigurationFile::SaveIdVal(config, id, val);
				}
			}
		}

		return config;
	}

	Configurations ReadConfigurations(std::string filePath) {
		std::fstream file;
		std::string line;
		Configurations cfgs;
		
		ConfigurationFile::OpenFileRead(file, filePath);

		while (std::getline(file, line)) {
			if (line != "") {
				if (line[0] == '[') {
					line = line.substr(1, line.size() - 2);
					Utils::toLowerCase(line);

					if (line == "program") {
						ConfigurationFile::ReadNextConfiguration(file, cfgs.programConfig);
					} else if (line == "camera") {
						CameraConfiguration config;
						ConfigurationFile::ReadNextConfiguration(file, config);

						if (config.noiseThreshold == 0) {
							std::cout << "[Warning] camera option \"noiseThreshold\" is 0, this can cause problems." << std::endl;
						}

						cfgs.camerasConfigs.push_back(config);
					}
				}
			}
		}
		
		ConfigurationFile::PreprocessConfigurations(cfgs);

		file.close();
		return cfgs;
	}

	void PreprocessConfigurations(Configurations& cfgs) {
		for (auto &&config : cfgs.camerasConfigs) {
			if (config.framesToAnalyze.framesBefore == nullptr)
				config.framesToAnalyze.framesBefore = new size_t(*cfgs.programConfig.numberGifFrames.framesBefore);
			
			if (config.framesToAnalyze.framesAfter == nullptr)
				config.framesToAnalyze.framesAfter = new size_t(*cfgs.programConfig.numberGifFrames.framesAfter);
		}	
	}

	void WriteConfigurationFileHeader(std::fstream& file) {
		std::ostringstream ss;
		
		ss << "; This file contains the configurations for each camera and the program itself. You can change it freely, following the sintaxis."
			<< "\n; The variables are defined with the following sintaxis: <id>=<value>"
			<< "\n; You can use lowercase, uppercase, mixed, it doesn't mind."
			<< "\n; The program configuration is  declared with [PROGRAM]."
			<< "\n; Each camera configuration begins with a [CAMERA] header and end where another [CAMERA] is found or EOF.\n\n";
			
		ConfigurationFile::WriteLineInFile(file, ss.str().c_str());
	}

	// Writes a camera configuration to the file
	void WriteConfiguration(std::fstream& file, CameraConfiguration& cfg) {
		std::ostringstream ss;
		
		// Write header
		ConfigurationFile::WriteLineInFile(file, "\n\n[CAMERA]");
		
		ss 	<< "\ncameraName=" << cfg.cameraName
			
			<< "\n\n; ";
			if (!cfg.url.empty())
				ss << "\nurl=" << cfg.url;
			else
				ss << "\n;url=" << cfg.url;
			
		ss	<< "\n\n; ROI (Regin of interest) crop each image that the camera sends. Sintaxis is: [(<p_x>,<p_y>),(<widht>,<height>)]"
			<< "\nroi=" <<  Utils::RoiToString(cfg.roi)
			
			<< "\n\n; "
			<< "\nhitThreshold=" <<  std::fixed << std::setprecision(2) << cfg.hitThreshold
				
			<< "\n\n; Order of the camera in the preview"
			<< "\norder=" << cfg.order
			
			<< "\n\n; Rotation (degrees)"
			<< "\nrotation=" << cfg.rotation;
			
		// frames to analyze
		ss << "\n\n; Selects the frame to search a person on. The sintaxis is: <nframesBefore>..<nframesAfter>."
			<< "\n; \"..\" denotes the frame where the change was detected (initial).";
		if (cfg.framesToAnalyze.framesBefore != nullptr && cfg.framesToAnalyze.framesAfter != nullptr)
			ss << "\nframesToAnalyze=" << *cfg.framesToAnalyze.framesBefore << ".." << *cfg.framesToAnalyze.framesAfter;
		else 
			ss << "\n;framesToAnalyze=<nframesBefore>..<nframesAfter>";
				
		ss	<< "\n\n; Disabled: Camera is disabled, doesn't show or process frames."
			<< "\n; Sentry: Only sends notifications."
			<< "\n; Active: Same as Sentry but try to recognize a person in the frames selected on \"framesToAnalyze\"."
			<< "\ntype=" << (cfg.type == CAMERA_DISABLED ? "Disabled" : (cfg.type == CAMERA_SENTRY ? "Sentry"  : "Active")) 
			
			<< "\n\n# == Change (ammount of pixels between the last two frames) section"
			
			<< "\n\n; Initial change threshold, will be updated after <updateThresholdFrequency> seconds."
			<< "\nchangeThreshold=" << cfg.changeThreshold
			
			<< "\n\n; Used to remove noise (single scattered pixels). Between 30 and 50 is a general good value."
			<< "\nthresholdNoise=" <<  std::fixed << std::setprecision(2) << cfg.noiseThreshold
			
			<< "\n\n; Minimum number of different pixels between the last 2 frames. Is used to leave a margin of \"error\"."
			<< "\n; Is recommended to set it at a low number, like 10."
			<< "You maybe will have to change it if you change the theshold noise or update Frequency"
			<< "\nminimumThreshold=" << cfg.minimumThreshold
			
			<< "\n\n; Since the app is calculating the average change of pixels between the last two images you need to leave a margin"
			<< "\n; to avoid sending notifications over small or insignificant changes."
			<< " A general good value is between 1.04 (4%) and 1.30 (30%) of the average change."
			<< "\nincreaseTresholdFactor=" <<  std::fixed << std::setprecision(2) << cfg.increaseTresholdFactor
			
			<< "\n\n; This tells the app how frequent (seconds) to update the average pixels change between the last two frames."
			<< "\n; On camera where there is fast changing objects is good to leave this value low, e.g. 5."
			<< "\nupdateThresholdFrequency=" << cfg.updateThresholdFrequency
			
			<< "\n\n# == Ignored areas section"
			
			<< "\n\n; How many objects or changes on ignored areas are needed in order to not send a notification about the change?"
			<< "\nthresholdFindingsOnIgnoredArea=" << cfg.thresholdFindingsOnIgnoredArea
			
			<< "\n\n; Maybe the object didn't match with all the ignored area, so is better to leave a margin for \"errors\"."
			<< "\n; Recommended value: between 90 and 100."
			<< "\nminPercentageAreaNeededToIgnore=" << cfg.minPercentageAreaNeededToIgnore;
		
		// ignored areas
		ss << "\n\n; List of ignored areas. Sintaxis: <p_x>,<p_y>,<widht>,<height>"
			<< "\n; Also you can use parentheses and brackets to make it more readable, e.g. [(16,25), (100,100)],[(100,150),(50,50)]";
		if (cfg.ignoredAreas.size() > 0)
			ss << "\nignoredAreas=" << Utils::IgnoredAreasToString(cfg.ignoredAreas);
		else 
			ss << "\n;ignoredAreas=";
			
			
		ss 	<< "\n\n# == Areas delimiters (NOT IN USE RIGHT NOW)."
			<< "\n\n; "
			<< "\nsecondsWaitEntryExit=" << cfg.secondsWaitEntryExit
			<< "\n\n; ";
			
		// area delimites (not in use)
		if (!cfg.areasDelimiters.rectEntry.empty() || !cfg.areasDelimiters.rectExit.empty())
			ss << "\nareasDelimiters=" << Utils::AreasDelimitersToString(cfg.areasDelimiters);
						
		ConfigurationFile::WriteLineInFile(file, ss.str().c_str());
	}

	void WriteConfiguration(std::fstream& file, ProgramConfiguration& cfg){
		std::ostringstream ss;

		// Write header
		ConfigurationFile::WriteLineInFile(file, "[PROGRAM]");
		
		ss 	<< "\n; Milliseconds between each frame. Greater <msBetweenFrame> = Lower CPU usage."
				<< "\n; FPS = 1000 / <msBetweenFrame>. Try different values and determine the efficiency vs effectiveness."
				<< "\nmsBetweenFrame=" << cfg.msBetweenFrame
				
				<< "\n\n# == Output-preview section\n"
				
				<< "\n; Comment the line below to let the program calculate the output res automatically"
				<< "\n;outputResolution=" << cfg.outputResolution.width << "," << cfg.outputResolution.height
				
				<< "\nratioScaleOutput=" << std::fixed << std::setprecision(2) << cfg.ratioScaleOutput
				<< "\nshowignoredareas=" << (cfg.showIgnoredAreas ?  "yes" : "no")
				<< "\nshowPreviewCameras=" << (cfg.showPreview ?  "yes" : "no")
				<< "\nshowAreaCameraSees=" << (cfg.showAreaCameraSees ?  "yes" : "no")
				<< "\nshowProcessedFrames=" << (cfg.showProcessedFrames ?  "yes" : "no")
				
				<< "\n\n# == Telegram and notifications section\n"
				
				<< "\ntelegramBotApi=" << cfg.telegramConfig.apiKey
				<< "\ntelegramChatId=" << cfg.telegramConfig.chatId
				<< "\nuseTelegramBot=" << (cfg.telegramConfig.useTelegramBot ? "yes" : "no")
				<< "\nsendimageofallcameras=" << (cfg.sendImageOfAllCameras ?  "yes" : "no")
				<< "\nsecondsBetweenImage=" << cfg.secondsBetweenImage
				<< "\nsecondsBetweenMessage=" << cfg.secondsBetweenMessage
				<< "\nsendImageWhenDetectChange=" << (cfg.sendImageWhenDetectChange ?  "yes" : "no")
				
				<< "\n\n; Authorized users to send actions from telegram. Sintaxis: user_1,user_2,...,user_n."
				<< "\nauthUsersToSendActions=" << Utils::VectorToCommaString(cfg.authUsersToSendActions)
				
				<< "\n\n; This tells the app if send a image or a gif."
				<< "\nuseGifInsteadOfImage=" << (cfg.useGifInsteadImage ?  "yes" : "no")
				
				<< "\n\n; Selects the Quality of the gif. Values are:"
				<< "\n; Value : Meaning" 
				<< "\n;  100  : NONE. Do not resize the frames."
				<< "\n;  80   : LOW. Lowers 20% the resolution."
				<< "\n;  60   : MEDIUM. Lowers 40% the resolution."
				<< "\n;  40   : HIGH. Lowers 60% the resolution."
				<< "\n;  20   : VERYHIGH. Lowers 80% the resolution."
				<< "\ngifResizePercentage=" << (int)cfg.gifResizePercentage;
				
		ss << "\n\n; How much frames are going to be on the GIF. The sintaxis is: <nframesBefore>..<nframesAfter>."
			<< "\n; \"..\" denotes the frame where the change was detected (initial)."
			<< "\n; Have in mind that the GIF will send <msBetweenFrame>*<nframesAfter> ms after the change (+ conversion and upload time).";
		if (cfg.numberGifFrames.framesBefore != nullptr && cfg.numberGifFrames.framesAfter != nullptr)
			ss << "\ngifFrames=" << *cfg.numberGifFrames.framesBefore << ".."  << *cfg.numberGifFrames.framesAfter;
		else
			ss << "\n;gifFrames=<nframesBefore>..<nframesAfter>";
			
		ConfigurationFile::WriteLineInFile(file, ss.str().c_str());
	}

	void SaveConfigurations(Configurations& cfgs, std::string filePath) {
		std::fstream file;
		
		ConfigurationFile::OpenFileWrite(file, filePath);

		ConfigurationFile::WriteConfigurationFileHeader(file);

		ConfigurationFile::WriteConfiguration(file, cfgs.programConfig);
		
		for (auto &camera : cfgs.camerasConfigs)
			ConfigurationFile::WriteConfiguration(file, camera);

		file.close();
	}

	/// <summary> Saves the given id and value into the corresponding member of the program config </summary>
	void SaveIdVal(ProgramConfiguration& config, std::string id, std::string value) {
		Utils::toLowerCase(id);

		if (id == "msbetweenframe" || id == "millisbetweenframe") {
			config.msBetweenFrame = (ushort)std::stoi(value);
		} else if (id == "secondsbetweenimage") {
			std::replace(value.begin(), value.end(), ',', '.');
			config.secondsBetweenImage = std::stoi(value);
		} else if (id == "secondsbetweenmessage") {
			config.secondsBetweenMessage = std::stoi(value);;
		} else if (id == "outputresolution") {
			size_t indx = value.find(",");
			if (indx > 0) {
				try {
					config.outputResolution.width = std::stoi(value.substr(0, indx));
					config.outputResolution.height = std::stoi(value.substr(indx + 1, value.size() - 1));
				} catch (std::invalid_argument e) {
					std::cout << "Invalid input at option \"outputResolution\" in config.ini header PROGRAM. Expected format: width,height.\n";
					std::getchar();
					std::exit(-1);
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
		} else if (id == "sendimageofallcameras" || id == "sendImageallcameras") {
			Utils::toLowerCase(value);
			config.sendImageOfAllCameras = value == "no" ? false : true;		
		} else if(id == "authuserstosendactions" || id == "authserssendactions"){
			config.authUsersToSendActions = std::move(Utils::SplitString(value, ","));
		} else if (id == "ratioscaleoutput") {
			config.ratioScaleOutput = std::stod(value);
		} else if (id == "usegifinsteadofimage" || id == "usegif") {
			Utils::toLowerCase(value);
			config.useGifInsteadImage = value == "no" ? false : true;
		} else if (id == "gifresizepercentage" || id == "gifresize") {
			if (value == "None") {
				config.gifResizePercentage = GifResizePercentage::None;
			} else if (value == "Low") {
				config.gifResizePercentage = GifResizePercentage::Low;
			} else if (value == "Medium") {
				config.gifResizePercentage = GifResizePercentage::Medium;
			} else if (value == "High") {
				config.gifResizePercentage = GifResizePercentage::High;
			} else if (value == "VeryHigh") {
				config.gifResizePercentage = GifResizePercentage::VeryHigh;
			}
		} else if (id == "gifframes") {
			std::vector<std::string> results = Utils::GetRange(value);
			size_t sz = results.size();
			if (sz == 3) {
				config.numberGifFrames.framesBefore = new size_t(std::stol(results[0]));
				config.numberGifFrames.framesAfter = new size_t(std::stol(results[2]));
			} else {
				std::cout << "Invalid field input \"gifframes\" in config.ini header PROGRAM. Expected format: totalFramesBefore..totalFramesAfter\n e.g. 5..60";
				std::getchar();
				std::exit(-1);
			}
		} else if (id == "showignoredareas") {
			Utils::toLowerCase(value);
			config.showIgnoredAreas = value == "no" ? false : true;
		} else {
			std::cout << "Campo: \"" <<  id << "\" no reconocido" << std::endl; 
		}
	}


	/// <summary> Saves the given id and value into the corresponding member of the camera config </summary>
	void SaveIdVal(CameraConfiguration& config, std::string id, std::string value) {
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
		} else if (id == "minimumthreshold") {
			int val = std::stoi(value);
			config.minimumThreshold = val;
		} else if (id == "increasetresholdfactor" || id == "increaseTreshold") {
			std::replace(value.begin(), value.end(), ',', '.');
			double val = std::stof(value);
			config.increaseTresholdFactor = val;
		} else if (id == "updatethresholdfrequency") {
			uint32_t val = std::stoi(value);
			config.updateThresholdFrequency = val;
		} else if (id == "usehighconstrast") {
			config.useHighConstrast = value == "yes";
		} else if (id == "ignoredareas"){
			std::vector<int> results = Utils::GetNumbersString(value);
			for (size_t i = 0; i < results.size() / 4; i++) {
				int base = i * 4;
				config.ignoredAreas.push_back(cv::Rect(cv::Point(results[base], results[base+1]), cv::Size(results[base+2], results[base+3])));			
			}	
		} else if (id == "framestoanalyze") {
			std::vector<std::string> results = Utils::GetRange(value);
			size_t sz = results.size();
			if (sz >= 1) {
				if (results[0] == "..") {
					if (sz >= 2)
						config.framesToAnalyze.framesAfter = new size_t(std::stol(results[1]));
				} else {
					config.framesToAnalyze.framesBefore = new size_t(std::stol(results[0]));

					if (sz >= 3)
						config.framesToAnalyze.framesAfter = new size_t(std::stol(results[2]));
				}
			}
		} else if (id == "thresholdfindingsonignoredarea") {
			int val = std::stoi(value);
			config.thresholdFindingsOnIgnoredArea = val;
		} else if (id == "minpercentageareaneededtoignore") {
			double val = std::stod(value);
			config.minPercentageAreaNeededToIgnore = val == 0 ? 0 : val / 100.0;
		} else {
			std::cout << "Campo: \"" <<  id << "\" no reconocido" << std::endl; 
		}
	}
}