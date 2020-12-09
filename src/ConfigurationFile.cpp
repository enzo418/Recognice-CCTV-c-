#include "ConfigurationFile.hpp"

const char* Configuration::GetFilePath(const char* fileName) {
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

void Configuration::OpenFile(){
	if (!_file.is_open()){ 
		char filename[MAX_PATH] = {};
		strcpy_s(filename, this->GetFilePath(_fileName));			
		if (Utils::FileExist(filename)) {
			_file.open(filename, std::fstream::in | std::fstream::out | std::ios::app);
		}
	}
}

inline void Configuration::WriteLineInFile(const char* line){
	_file.write(line, sizeof(char) * strlen(line));
}

Configuration::Configuration() {
	this->OpenFile();
};

Configuration::Configuration(const char* filePath) : _fileName(filePath) {
	this->OpenFile();
};

void Configuration::Read(const char* filePath) {
	this->_fileName = filePath;
	this->OpenFile();
	this->ReadConfigurations();
};

template<typename T>
T Configuration::ReadNextConfiguration(std::fstream& file, T& config) {
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
				this->SaveIdVal(config, id, val);
			}
		}
	}

	return config;
}

void Configuration::ReadConfigurations() {
	std::string line;

	while (std::getline(_file, line)) {
		if (line != "") {
			if (line[0] == '[') {
				line = line.substr(1, line.size() - 2);
				Utils::toLowerCase(line);

				if (line == "program") {
					this->ReadNextConfiguration(_file, this->configurations.programConfig);
				} else if (line == "camera") {
					CameraConfiguration config;
					this->ReadNextConfiguration(_file, config);

					// validate config
					if (config.type == CAMERA_DISABLED) {
						std::cout << config.cameraName << " skiped because its type is disabled" << std::endl;
					} else if (config.url.empty() /* check if is a valid url*/) {
						std::cout << config.cameraName << " skiped because its url is not valid" << std::endl;
					} else {
						if (config.noiseThreshold == 0) {
							std::cout << "[Warning] camera option \"noiseThreshold\" is 0, this can cause problems." << std::endl;
						}

						this->configurations.camerasConfigs.push_back(config);
					}
				}
			}
		}
	}

	_file.close();
}

void Configuration::PreprocessConfigurations() {
	for (auto &&config : this->configurations.camerasConfigs) {
		if (config.framesToAnalyze.framesBefore == nullptr)
			config.framesToAnalyze.framesBefore = new size_t(*this->configurations.programConfig.numberGifFrames.framesBefore);
		
		if (config.framesToAnalyze.framesAfter == nullptr)
			config.framesToAnalyze.framesAfter = new size_t(*this->configurations.programConfig.numberGifFrames.framesAfter);
	}	
}

// Writes a camera configuration to the file
void Configuration::WriteCameraConfiguration(CameraConfiguration& cfg) {	
	this->OpenFile();

	std::string tmp;

	// Write header
	this->WriteLineInFile("\n[CAMERA]");

	tmp = "\ncameraName=" + cfg.cameraName;
	this->WriteLineInFile(tmp.c_str());

	tmp = "\norder=" + cfg.order;
	this->WriteLineInFile(tmp.c_str());

	tmp = "\nurl=" + cfg.url;
	this->WriteLineInFile(tmp.c_str());

	tmp = "\nrotation=" + cfg.rotation;
	this->WriteLineInFile(tmp.c_str());

	tmp = "\ntype=" + cfg.type;
	this->WriteLineInFile(tmp.c_str());

	std::ostringstream strs;
	strs << cfg.hitThreshold;
	tmp = "\nhitThreshold=" + strs.str();                
	this->WriteLineInFile(tmp.c_str());

	tmp = "\nchangeThreshold=" + cfg.changeThreshold;
	this->WriteLineInFile(tmp.c_str());

	tmp = "\nroi=" + Utils::RoiToString(cfg.roi);
	this->WriteLineInFile(tmp.c_str());

	strs.clear();
	strs << cfg.noiseThreshold;
	tmp = "\nthresholdNoise=" + strs.str();  
	this->WriteLineInFile(tmp.c_str());

	tmp = "\nsecondsWaitEntryExit=" + cfg.secondsWaitEntryExit;
	this->WriteLineInFile(tmp.c_str());

	tmp = "\nareasDelimiters=" + Utils::AreasDelimitersToString(cfg.areasDelimiters);
	this->WriteLineInFile(tmp.c_str());	
}

void Configuration::WriteProgramConfiguration(){
	
}

void Configuration::SaveConfigurations() {
	this->WriteProgramConfiguration();

	for (auto &camera : this->configurations.camerasConfigs)	
		this->WriteCameraConfiguration(camera);	
}

/// <summary> Saves the given id and value into the corresponding member of the program config </summary>
void Configuration::SaveIdVal(ProgramConfiguration& config, std::string id, std::string value) {
	Utils::toLowerCase(id);

	if (id == "msbetweenframe" || id == "millisbetweenframe") {
		config.msBetweenFrame = (ushort)std::stoi(value);
	} else if (id == "secondsbetweenimage") {
		std::replace(value.begin(), value.end(), ',', '.');
		config.secondsBetweenImage = std::stof(value);
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
	} else {
		std::cout << "Campo: \"" <<  id << "\" no reconocido" << std::endl; 
	}
}


/// <summary> Saves the given id and value into the corresponding member of the camera config </summary>
void Configuration::SaveIdVal(CameraConfiguration& config, std::string id, std::string value) {
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
	} else {
		std::cout << "Campo: \"" <<  id << "\" no reconocido" << std::endl; 
	}
}

void Configuration::StartConfiguration() {
	std::string input;
	char configsText[] = "3. Exit configuration";
	while(input != "3" || this->configurations.camerasConfigs.size() == 0){
		while (input.empty()) {
			std::cout << "# Configuration" << std::endl
				<< "\t1. Cameras" << std::endl
				<< "\t2. Program" << std::endl;
			
			if(this->configurations.camerasConfigs.size() > 0) std::cout << configsText << std::endl;

			std::cout << "- Chose a option: ";

			std::getline(std::cin, input);
		}

		if (input == "1") {
			this->StartCameraConfiguration();
		} else if (input == "2") {
			this->StartProgramConfiguration();
		}
	}
}

void Configuration::StartProgramConfiguration(){
	// TODO
}

void Configuration::StartCameraConfiguration(){
	std::string input;
	
	while (input != "3")
	{
		while (input.empty()) {
			std::cout << "\n# Cameras configurations" << std::endl
				<< "\t1. Add new camera" << std::endl
				<< "\t2. Set the Area Delimiters of a camera" << std::endl
				<< "\t3. Back" << std::endl
				<< "- Chose a option: ";

			std::getline(std::cin, input);
		}

		if (input == "1") {
			this->ReadNewCamera();			
		} else if (input == "2") {
			this->SetAreaDelimitersCamera();
		}
		
		input.clear();
	}
}

void Configuration::ReadNewCamera() {
	CameraConfiguration config;

	this->LoadConfigCamera(config, config, false);

	this->WriteCameraConfiguration(config);

	this->configurations.camerasConfigs.push_back(config);
}

void Configuration::LoadConfigCamera(CameraConfiguration& src, CameraConfiguration& dst, bool isModification = false){
	std::string input;        
	
	if (isModification)
		std::cout << "\tNote: if you want to leave te original value [value], then just press enter when the program ask you to enter it."
		<< std::endl;

	while (1) {
		std::cout << "Enter the camera name: ";
		if (isModification) std::cout << "[" << src.cameraName << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.cameraName = input;

		std::cout << "Enter the camera url: ";
		if (isModification) std::cout << "[" << src.url << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.url = input;

		std::cout << "Enter the camera roi";
		if (isModification) std::cout << "[" << Utils::RoiToString(src.roi) << "] ";
		else std::cout << ", format [(x1,y1),(x2,y2)]: ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.roi = Utils::GetROI(input);

		std::cout << "Enter camera order: ";
		if (isModification) std::cout << "[" << src.order << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.order = std::stoi(input);

		std::cout << "Enter rotation (deg): ";
		if (isModification) std::cout << "[" << src.rotation << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.rotation = std::stoi(input);

		std::cout << "Enter camera change threshold: ";
		if (isModification) std::cout << "[" << src.changeThreshold << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.changeThreshold = std::stoi(input);

		std::cout << "Enter the camera type Sentry, Active, Disabled: ";
		if (isModification) std::cout << "[" << Utils::CamTypeToString(src.type) << "] ";
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
		if (isModification) std::cout << "[" << src.hitThreshold << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			this->SaveIdVal(dst, "hitthreshold", input);

		std::cout << "Enter the noise threshold: ";
		if (isModification) std::cout << "[" << src.noiseThreshold << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			this->SaveIdVal(dst, "noisethreshold", input);
				
		std::cout 	<< "The program will show you a window where you will nedd to select: "
					<<	"\n [YELLOW] The area where if a person is seen in it, they will be marked as entering the site."
					<<	"\n [GREEN]  The area where if a person is seen in it, they will be marked as leaving the site."
					<<	"\n Use LeftMouseClick or LeftMouseClick to set the set the entry or exit point and then Move the mouse to select the area." 
					<< std::endl;
		// call the f
		dst.areasDelimiters = this->StartConfigurationAreaEntryExit(dst);

		std::cout << "Selected areas => " << Utils::AreasDelimitersToString(dst.areasDelimiters) << std::endl;

		std::cout << "Enter the time (in seconds) to wait from the time a person reaches the point of entry or "
					<< "exit until they reach the other point. After this time the program forgets about that person: ";
		if (isModification) std::cout << "[" << src.secondsWaitEntryExit << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.secondsWaitEntryExit = std::stoi(input);

		std::cout << "Exit and save changes? (y/N):";
		std::getline(std::cin, input);
		Utils::toLowerCase(input);
		if (input == "yes" || input == "y") {
			src = dst;			
			return;
		} else {
			std::cout << "Edit again? (yes/no):";
			std::getline(std::cin, input);
			if (input == "no" || input == "NO") return;
		}
	}
}

AreasDelimiters Configuration::StartConfigurationAreaEntryExit(CameraConfiguration& config){
	this->areaConfig = new AreaEntryExitConfig;

	cv::namedWindow("Press a key to exit", 0);

    cv::setMouseCallback("Press a key to exit", Configuration::onMouse, this->areaConfig);

    cv::VideoCapture capture(config.url);
    capture.read(this->areaConfig->img);

    cv::resize(this->areaConfig->img, this->areaConfig->img, RESIZERESOLUTION);

    if (!config.roi.isEmpty()) {
        cv::Rect roi(config.roi.point1, config.roi.point2);
        this->areaConfig->img = this->areaConfig->img(roi);
    }
    
    cv::imshow("Press a key to exit", this->areaConfig->img);

    cv::waitKey(0);

	cv::destroyAllWindows();

	AreasDelimiters area = AreasDelimiters(this->areaConfig->lastRectEntry, this->areaConfig->lastRectExit);

	delete this->areaConfig;

    return area;
}

void Configuration::SetAreaDelimitersCamera(){
	std::cout << "\n# Set Area Delimiters. Active cameras: \n";
	std::string configsOpts = "(";
	std::string input;

	size_t size = this->configurations.camerasConfigs.size();

	for (size_t i = 0; i < size; i++) {
		configsOpts += std::to_string(i+1) + (i != size-1 ? "," : ")");		

		std::cout << "\t" << i+1 << ". " << this->configurations.camerasConfigs[i].cameraName << std::endl;
		std::cout << "\t  " << " url=" << this->configurations.camerasConfigs[i].url << std::endl;
		if(this->configurations.camerasConfigs[i].areasDelimiters.rectEntry.empty() && this->configurations.camerasConfigs[i].areasDelimiters.rectExit.empty()){
			std::cout << "\t  " << " areasDelimiters not found" << std::endl;
		}else{
			std::cout << "\t  " << " areaDelimiters=" << Utils::AreasDelimitersToString(this->configurations.camerasConfigs[i].areasDelimiters) << std::endl;
		}
	}

	std::cout 	<< "Before you select a config know that the program will show you a window where you will nedd to select: "
				<<	"\n [YELLOW] The area where if a person is seen in it, they will be marked as entering the site."
				<<	"\n [GREEN]  The area where if a person is seen in it, they will be marked as leaving the site."
				<<	"\n Use LeftMouseClick or LeftMouseClick to set the set the entry or exit point and then Move the mouse to select the area." 
				<< std::endl;

	while (input.empty()) {
		std::cout << "- Please enter a camera config "<< configsOpts << ":";
		std::getline(std::cin, input);
	}

	int indx = std::stoi(input) - 1;

	if (indx >= 0 && indx < size) {
		AreasDelimiters del = StartConfigurationAreaEntryExit(this->configurations.camerasConfigs[indx]);
		if(del.rectEntry.empty() && del.rectExit.empty()) std::cout << "The areas are empty." << std::endl;
		this->configurations.camerasConfigs[indx].areasDelimiters = del;
		
		std::cout << "Selected areas has been updated => " << Utils::AreasDelimitersToString(this->configurations.camerasConfigs[indx].areasDelimiters) 
					<< "\n Copy that to the corresponding camera configuration in the configuration file to save it."
					<< std::endl;

	} else std::cout << "ERROR: Invalid index " << indx << std::endl;
}

void Configuration::onMouse(int event, int x, int y, int flags, void* params){
 	AreaEntryExitConfig* areaConfig = (AreaEntryExitConfig*)params;

	cv::Point point(x, y);
    areaConfig->dst = areaConfig->img.clone();
    bool updateImg = false;

    if (event == cv::EVENT_LBUTTONDOWN){ // EVENT_LBUTTONDOWN
        areaConfig->lastEvent = cv::EVENT_LBUTTONDOWN;        
        areaConfig->entryPoint1 = point;
    } else if (event == cv::EVENT_RBUTTONDOWN){ // EVENT_RBUTTONDOWN
        areaConfig->lastEvent = cv::EVENT_RBUTTONDOWN;
        areaConfig->exitPoint1 = point;
    } else if (event == cv::EVENT_LBUTTONUP || event == cv::EVENT_RBUTTONUP){
        areaConfig->lastEvent = -1;
        updateImg = false;
    } else if (event == cv::EVENT_MOUSEMOVE){
        if (areaConfig->lastEvent != -1){
            cv::Point point1 = areaConfig->lastEvent == cv::EVENT_RBUTTONDOWN ? areaConfig->exitPoint1 : areaConfig->entryPoint1;
            cv::Scalar color;
            cv::Rect rect = cv::Rect(cv::Point(point1.x - point.x / 2, point1.y - point.y / 2), cv::Size(point));

            if (areaConfig->lastEvent == cv::EVENT_RBUTTONDOWN){
                areaConfig->exitPoint2 = point;
                color = areaConfig->colorExit;
                areaConfig->lastRectExit = rect;
                if(!areaConfig->lastRectEntry.empty()) cv::rectangle(areaConfig->dst, areaConfig->lastRectEntry, areaConfig->colorEntry, areaConfig->radius/10);
            } else if (areaConfig->lastEvent == cv::EVENT_LBUTTONDOWN){
                areaConfig->entryPoint2 = point;
                color = areaConfig->colorEntry;
                areaConfig->lastRectEntry = rect;
                if(!areaConfig->lastRectExit.empty()) cv::rectangle(areaConfig->dst, areaConfig->lastRectExit, areaConfig->colorExit, areaConfig->radius/10);
            }

            cv::rectangle(areaConfig->dst, rect, color, areaConfig->radius/10);
            updateImg = true;
        }
    }

    if(updateImg){
        cv::imshow("Press a key to exit", areaConfig->dst);
    }
}