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
			file.open(filename, std::ofstream::out | std::ofstream::trunc);
		}
	}

	inline void WriteLineInFile(std::fstream& file, const char* line){
		file.write(line, sizeof(char) * strlen(line));
	}

	template<typename T>
	T ReadNextConfiguration(std::istream& file, T& config, std::string& error) {
		std::string line;
		size_t lineNumber = 0;

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
					if (!ConfigurationFile::SaveIdVal(config, id, val)) {
						std::ostringstream ss;
						ss 	<< "Invalid field of value in line: " << lineNumber 
							<< ". Field: '" << id << "' value: '" << val << "'";
						error = ss.str();						
						break;
					}
				}
			}
			lineNumber++;
		}

		return config;
	}

	Configurations ReadConfigurations(std::string filePath, std::string& error) {
		std::fstream file;
		
		ConfigurationFile::OpenFileRead(file, filePath);

		Configurations cfgs = ReadConfigurationBuffer(file, error);

		file.close();
		return cfgs;
	}

	// template <typename S>
	Configurations ReadConfigurationBuffer(std::istream& cfgBuffer, std::string& error) {		
		std::string line;
		Configurations cfgs;
		while (std::getline(cfgBuffer, line) && error.length() == 0) {
			if (line != "") {
				if (line[0] == '[') {
					line = line.substr(1, line.size() - 2);
					Utils::toLowerCase(line);

					if (line == "program") {
						ConfigurationFile::ReadNextConfiguration(cfgBuffer, cfgs.programConfig, error);
					} else if (line == "camera") {
						CameraConfiguration config;
						ConfigurationFile::ReadNextConfiguration(cfgBuffer, config, error);

						if (config.noiseThreshold == 0) {
							std::cout << "[Warning] camera option \"noiseThreshold\" is 0, this can cause problems." << std::endl;
						}

						cfgs.camerasConfigs.push_back(config);
					}
				}
			}
		}
		
		if (error.length() == 0)
			ConfigurationFile::PreprocessConfigurations(cfgs);

		return cfgs;
	}

	void PreprocessConfigurations(Configurations& cfgs) {
		for (auto &&config : cfgs.camerasConfigs) {
			if (config.framesToAnalyze.framesBefore == -1)
				config.framesToAnalyze.framesBefore = cfgs.programConfig.numberGifFrames.framesBefore;
			
			if (config.framesToAnalyze.framesAfter == -1)
				config.framesToAnalyze.framesAfter = cfgs.programConfig.numberGifFrames.framesAfter;
		}	
	}

	std::string ConfigurationsToString(Configurations& cfgs) {
		std::string pConf = ConfigurationFile::GetConfigurationString(cfgs.programConfig);
		std::string camerasConf = ConfigurationFile::GetConfigurationString(cfgs.camerasConfigs);
		
		return pConf + camerasConf;
	}

	std::string GetConfigurationFileHeaderString() {
		std::ostringstream ss;
		
		ss << "; This file contains the configurations for each camera and the program itself. You can change it freely, following the syntax."
			<< "\n; The variables are defined with the following syntax: <id>=<value>"
			<< "\n; You can use lowercase, uppercase, mixed, it doesn't mind."
			<< "\n; Some variables need boolean values, 1 is true and 0 is false."
			<< "\n; The program configuration is  declared with [PROGRAM]."
			<< "\n; Each camera configuration begins with a [CAMERA] header and end where another [CAMERA] is found or EOF.\n\n";
		
		return ss.str();
	}

	std::string GetDocumentationString() {
		std::ostringstream ss;

	ss 		<< "\n; Variables will be explained with a comment above it and the syntax or type expected after ="
			<< "\n; type can be: \n;\t - string\n;\t - number: integer (int) or decimal\n;\t - boolean: 1 for ON or 0 for OFF"
			
			/* Program doc */
			<< "\n\n;[PROGRAM]"
			<< "\n; Milliseconds between each frame. Greater <msBetweenFrame> = Lower CPU usage."
			<< "\n; FPS = 1000 / <msBetweenFrame>. Try different values and determine the efficiency vs effectiveness."
			<< "\n;msBetweenFrame=int"
			
			<< "\n\n; Same as <msBetweenFrame>. But only will be applied after detecting a change in the frames."
			<< "\n;msBetweenFrameAfterChange=int"
			
			<< "\n\n# == Output-preview section\n"
			
			<< "\n; Comment the line below to let the program calculate the output res automatically"
			<< "\n;outputResolution=number,number"
			
			<< "\n;ratioScaleOutput=decimal"
			<< "\n;showignoredareas=boolean"
			<< "\n;showPreviewCameras=boolean"
			<< "\n;showAreaCameraSees=boolean"
			<< "\n;showProcessedFrames=boolean"
			
			<< "\n\n# == Telegram and notifications section\n"
			<< "\n;telegramBotApi=string"
			<< "\n;telegramChatId=string"
				
			<< "\n;useTelegramBot=boolean"
			<< "\n;sendimageofallcameras=boolean"
			<< "\n;secondsBetweenImage=int"
			<< "\n;secondsBetweenMessage=int"
			<< "\n;telegrambot_sendImageWhenDetectChange=boolean"
			<< "\n;telegrambot_sendGifWhenDetectChange=boolean"
			<< "\n;telegrambot_sendTextWhenDetectChange=boolean"

			<< "\n;localnotifications_sendTextWhenDetectChange=boolean"
			<< "\n;localnotifications_sendGifWhenDetectChange=boolean"
			<< "\n;localnotifications_sendImageWhenDetectChange=boolean"
			<< "\n;localnotifications_sendVideoWhenDetectChange=boolean"
			
			<< "\n\n; Authorized users to send actions from telegram. syntax: user_1,user_2,...,user_n."
			<< "\n;authUsersToSendActions=string,string,..."
			
			<< "\n\n; This tells the app if send a image or a gif."
			<< "\n;useGifInsteadOfImage=boolean"

			<< "\n\n; Should the program analyze pre- and post-change images in order to verify the validity of the change?"
			<< "\n; If active, the program will verify if each change is in an ignored area."
			<< "\n; If not active, the program will just verify if the detected change is not in an ignored area."
			<< "\n;analizeBeforeAfterChangeFrames=boolean"

			<< "\n\n; If the program should draw on the notification the trace of the change found on: \n;  - 0: none\n;  - 1: image\n;  - 2: gif\n;  - 3: both"
			<< "\n;drawTraceOfChangeFoundOn=int"

			<< "\n\n; Should the program save the GIF into a .avi (video) file?"
			<< "\n;saveChangeInVideo=boolean"
			
			<< "\n\n; Should the program draw the change found?"
			<< "\n;drawChangeFoundBetweenFrames=boolean"

			<< "\n\n; If so, then how much frames before and after should be considered."
			<< "\n; The syntax is: <nframesBefore>..<nframesAfter>."
			<< "\n; \"..\" denotes the frame where the change was detected (initial)."
			<< "\n;framesToAnalyzeChangeValidity=integer..integer"

			<< "\n\n; Selects the Quality of the gif. Values go from 0 to 100. 50 is means that the gif will be resized at half the resolution."
			<< "\n;gifResizePercentage=int"
			
			<< "\n\n; Select the detection method"
			<< "\n; 0: Do not try detect a person."
			<< "\n; 1: HOG Descriptor, uses built in opencv HOG Descriptor."
			<< "\n; 2: YOLO V4 DNN, uses darknet neural net, more precise than HOG."
			<< "\n;detectionMethod=int"
			
			<< "\n\n; How much frames are going to be on the GIF. The syntax is: <nframesBefore>..<nframesAfter>."
			<< "\n; \"..\" denotes the frame where the change was detected (initial)."
			<< "\n; Have in mind that the GIF will send <msBetweenFrame>*<nframesAfter> ms after the change (+ conversion and upload time)."
			<< "\n;gifFrames=integer..integer"

			<< "\n\n; Folder where the images will be saved"
			<< "\n;imagesFolder=string"

			/* Camera doc*/
			<< "\n\n;[CAMERA]"
			<< "\n;cameraName=string"
			<< "\n;url=string"
			<< "\n\n; ROI (Region of interest) crop each image that the camera sends. syntax is: <p_x>,<p_y>,<widht>,<height>"
			<< "\n;roi=integer,integer,integer,integer"
			
			<< "\n\n; "
			<< "\n;hitThreshold=decimal"
				
			<< "\n\n; Order of the camera in the preview"
			<< "\n;order=integer"
			
			<< "\n\n; Rotation (degrees)"
			<< "\n;rotation=integer"
			
			<< "\n\n; Selects the frame to search a person on. The syntax is: <nframesBefore>..<nframesAfter>."
			<< "\n; \"..\" denotes the frame where the change was detected (initial)."
			<< "\n;framesToAnalyze=integer..integer"
				
			<< "\n\n; Value : Meaning"
			<< "\n;   0   : Disabled, Camera is disabled, doesn't show or process frames."
			<< "\n;   1   : Sentry, Only sends notifications."
			<< "\n;   2   : Active, Same as Sentry but try to recognize a person in the frames selected on \"framesToAnalyze\"."
			<< "\n;type=integer"
			
			<< "\n\n# == Change (ammount of pixels between the last two frames) section"
			
			<< "\n\n; Used to remove noise (single scattered pixels). Between 30 and 50 is a general good value."
			<< "\n;thresholdNoise=decimal"
			
			<< "\n\n; Minimum number of different pixels between the last 2 frames. Is used to leave a margin of \"error\"."
			<< "\n; Is recommended to set it at a low number, like 10."
			<< "You maybe will have to change it if you change the theshold noise or update Frequency"
			<< "\n;minimumThreshold=integer"
			
			<< "\n\n; Since the app is calculating the average change of pixels between the last two images you need to leave a margin"
			<< "\n; to avoid sending notifications over small or insignificant changes."
			<< " A general good value is between 1.04 (4%) and 1.30 (30%) of the average change."
			<< "\n;increaseTresholdFactor=decimal"
			
			<< "\n\n; This tells the app how frequent (seconds) to update the average pixels change between the last two frames."
			<< "\n; On camera where there is fast changing objects is good to leave this value low, e.g. 5."
			<< "\n;updateThresholdFrequency=integer"
			
			<< "\n\n# == Ignored areas section"
			
			<< "\n\n; How many objects or changes on ignored areas are needed in order to not send a notification about the change?"
			<< "\n;thresholdFindingsOnIgnoredArea=integer"
			
			<< "\n\n; Maybe the object didn't match with all the ignored area, so is better to leave a margin for \"errors\"."
			<< "\n; Recommended value: between 90 and 100."
			<< "\n;minPercentageAreaNeededToIgnore=integer"
		
			<< "\n\n; List of ignored areas. syntax: <p_x>,<p_y>,<widht>,<height>"
			<< "\n; Also you can use parentheses and brackets to make it more readable, e.g. [(16,25), (100,100)],[(100,150),(50,50)]"
			<< "\n;ignoredAreas=integer,integer,integer,integer,..."

			<< "\n\n; List of polygon that describes a area discriminator. syntax: (allowed|deny):p11_x, p11_y, ..., p1n_x, p1n_y-(allowed|deny):p21_x, p21_y, ..., p2n_x, p2n_y-..."
			<< "\n;pointsdiscriminators=string";

			// << "\n\n# == Areas delimiters (NOT IN USE RIGHT NOW)."
			// << "\n\n; "
			// << "\nsecondsWaitEntryExit=" << cfg.secondsWaitEntryExit
			// << "\n\n; ";
		
		return ss.str();
	}

	// Writes a camera configuration to the file
	std::string GetConfigurationString(CameraConfiguration& cfg) {
		std::ostringstream ss;
			
		ss 	<< "\n\n[CAMERA]";

		if (!cfg.cameraName.empty())
			ss 	<< "\ncameraName=" << cfg.cameraName;
		
		if (!cfg.url.empty())
			ss << "\nurl=" << cfg.url;
					
		ss	<< "\nroi=" << Utils::RectToCommaString(cfg.roi)
			
			<< "\nhitThreshold=" <<  std::fixed << std::setprecision(2) << cfg.hitThreshold
						
			<< "\norder=" << cfg.order
						
			<< "\nrotation=" << cfg.rotation;
			
		if (cfg.framesToAnalyze.framesBefore != -1 && cfg.framesToAnalyze.framesAfter != -1)
			ss << "\nframesToAnalyze=" << cfg.framesToAnalyze.framesBefore << ".." << cfg.framesToAnalyze.framesAfter;
				
		ss	<< "\ntype=" << (int)cfg.type
					
			<< "\nthresholdNoise=" <<  std::fixed << std::setprecision(2) << cfg.noiseThreshold
			
			<< "\nminimumThreshold=" << cfg.minimumThreshold
			
			<< "\nincreaseTresholdFactor=" <<  std::fixed << std::setprecision(2) << cfg.increaseTresholdFactor
			
			<< "\nupdateThresholdFrequency=" << cfg.updateThresholdFrequency
					
			<< "\nthresholdFindingsOnIgnoredArea=" << cfg.thresholdFindingsOnIgnoredArea
			
			<< "\nminPercentageAreaNeededToIgnore=" << cfg.minPercentageAreaNeededToIgnore
			
			<< "\nminPercentageInsideAllowDiscriminator=" << cfg.minPercentageInsideAllowDiscriminator

			<< "\nmaxPercentageInsideDenyDiscriminator=" << cfg.maxPercentageInsideDenyDiscriminator;
		
		if (cfg.ignoredAreas.size() > 0)
			ss << "\nignoredAreas=" << Utils::IgnoredAreasToString(cfg.ignoredAreas);
		
		if (cfg.pointDiscriminators.size() > 0)
			ss << "\npointsDiscriminators=" << Utils::DiscriminatorArea2String(cfg.pointDiscriminators);
			
		return ss.str();
	}

	std::string GetConfigurationString(ProgramConfiguration& cfg) {
		std::ostringstream ss;

		std::string& messageOnTextNotification = cfg.messageOnTextNotification;
		Utils::trim(messageOnTextNotification);
		messageOnTextNotification = messageOnTextNotification.size() > 0 ? messageOnTextNotification : "Movement on camera ${N}";
		
		// Write header
		ss  << "\n\n[PROGRAM]"
			<< "\nmsBetweenFrame=" << cfg.msBetweenFrame
				
			<< "\nmsBetweenFrameAfterChange=" << cfg.msBetweenFrameAfterChange
			
			<< "\noutputResolution=" << cfg.outputResolution.width << "," << cfg.outputResolution.height
			
			<< "\nratioScaleOutput=" << std::fixed << std::setprecision(2) << cfg.ratioScaleOutput
			<< "\nshowIgnoredAreas=" << (cfg.showIgnoredAreas ?  "1" : "0")
			<< "\nshowPreviewCameras=" << (cfg.showPreview ?  "1" : "0")
			<< "\nshowAreaCameraSees=" << (cfg.showAreaCameraSees ?  "1" : "0")
			<< "\nshowProcessedFrames=" << (cfg.showProcessedFrames ?  "1" : "0");

			if (!cfg.telegramConfig.apiKey.empty())
				ss << "\ntelegramBotApi=" << cfg.telegramConfig.apiKey;

			if (!cfg.telegramConfig.chatId.empty())
				ss << "\ntelegramChatId=" << cfg.telegramConfig.chatId;
					
		ss	<< "\n\nuseTelegramBot=" << (cfg.telegramConfig.useTelegramBot ? "1" : "0")
			<< "\nsendImageOfAllCameras=" << (cfg.sendImageOfAllCameras ?  "1" : "0")
			<< "\nsecondsBetweenImage=" << cfg.secondsBetweenImage
			<< "\nsecondsBetweenMessage=" << cfg.secondsBetweenMessage
			<< "\ntelegrambot_sendImageWhenDetectChange=" << (cfg.telegramConfig.sendImageWhenDetectChange ?  "1" : "0")
			<< "\ntelegrambot_sendGifWhenDetectChange=" << (cfg.telegramConfig.sendGifWhenDetectChange ?  "1" : "0")
			<< "\ntelegrambot_sendTextWhenDetectChange=" << (cfg.telegramConfig.sendTextWhenDetectChange ?  "1" : "0")
			<< "\ntelegrambot_sendVideoWhenDetectChange=" << (cfg.telegramConfig.sendVideoWhenDetectChange ?  "1" : "0")
			
			<< "\n\nimagesFolder=" << cfg.imagesFolder
			
			<< "\n\nuseLocalNotifications=" << (cfg.localNotificationsConfig.useLocalNotifications ? "1" : "0")
			<< "\nlocalnotifications_sendTextWhenDetectChange=" << (cfg.localNotificationsConfig.sendTextWhenDetectChange ?  "1" : "0")
			<< "\nlocalnotifications_sendGifWhenDetectChange=" << (cfg.localNotificationsConfig.sendGifWhenDetectChange ?  "1" : "0")
			<< "\nlocalnotifications_sendImageWhenDetectChange=" << (cfg.localNotificationsConfig.sendImageWhenDetectChange ?  "1" : "0")
			<< "\nlocalnotifications_sendVideoWhenDetectChange=" << (cfg.localNotificationsConfig.sendVideoWhenDetectChange ?  "1" : "0")
			
			<< "\n\nanalizeBeforeAfterChangeFrames=" << (cfg.analizeBeforeAfterChangeFrames ? "1" : "0")

			<< "\n\nsaveChangeInVideo=" << (cfg.saveChangeInVideo ? "1" : "0")

			<< "\n\ndrawChangeFoundBetweenFrames=" << (cfg.drawChangeFoundBetweenFrames ? "1" : "0")

			<< "\n\nmessageOnTextNotification=" << messageOnTextNotification

			<< "\n\ndrawTraceOfChangeFoundOn="  << (int)cfg.drawTraceOfChangeFoundOn;

			if (!cfg.authUsersToSendActions.empty())
				ss << "\nauthUsersToSendActions=" << Utils::VectorToCommaString(cfg.authUsersToSendActions);			
				
			ss 	<< "\ngifResizePercentage=" << cfg.gifResizePercentage
								
				<< "\ndetectionMethod=" << cfg.detectionMethod;
			
		if (cfg.numberGifFrames.framesBefore != -1 && cfg.numberGifFrames.framesAfter != -1)
			ss << "\ngifFrames=" << cfg.numberGifFrames.framesBefore << ".."  << cfg.numberGifFrames.framesAfter;

		if (cfg.framesToAnalyzeChangeValidity.framesBefore != -1 && cfg.framesToAnalyzeChangeValidity.framesAfter != -1)
			ss << "\nframesToAnalyzeChangeValidity=" << cfg.framesToAnalyzeChangeValidity.framesBefore << ".."  << cfg.framesToAnalyzeChangeValidity.framesAfter;

		return ss.str();
	}

	std::string GetConfigurationString(CamerasConfigurations& cfg) {
		std::string config;
		for (auto &camera : cfg) {
			// Get and Write configuration
			config += ConfigurationFile::GetConfigurationString(camera);
		}
		return config;
	}

	void SaveConfigurations(Configurations& cfgs, std::string filePath) {
		std::fstream file;
		
		ConfigurationFile::OpenFileWrite(file, filePath);
		
		// Get and Write configuration header
		ConfigurationFile::WriteLineInFile(file, ConfigurationFile::GetConfigurationFileHeaderString().c_str());
		
		// Get and Write documentation
		ConfigurationFile::WriteLineInFile(file, ConfigurationFile::GetDocumentationString().c_str());
			
		// Get and Write program configuration
		ConfigurationFile::WriteLineInFile(file, ConfigurationFile::GetConfigurationString(cfgs.programConfig).c_str());
		
		for (auto &camera : cfgs.camerasConfigs) {
			// Get and Write configuration
			ConfigurationFile::WriteLineInFile(file, ConfigurationFile::GetConfigurationString(camera).c_str());
		}

		file.close();
	}

	/// <summary> Saves the given id and value into the corresponding member of the program config </summary>
	bool SaveIdVal(ProgramConfiguration& config, std::string id, std::string value) {
		bool sucess = true;
		Utils::toLowerCase(id);

		if (id == "msbetweenframe" || id == "millisbetweenframe") {
			config.msBetweenFrame = (ushort)std::stoi(value);
		} else if (id == "msbetweenframeafterchange") {
			config.msBetweenFrameAfterChange = (ushort)std::stoi(value);
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
					sucess = false;
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
			config.showPreview = value == "0" ? false : true;
		} else if (id == "showareacamerasees" || id == "showareacamera") {
			Utils::toLowerCase(value);
			config.showAreaCameraSees = value == "0" ? false : true;
		} else if (id == "showprocessedframes" || id == "showprocessedimages") {
			Utils::toLowerCase(value);
			config.showProcessedFrames = value == "0" ? false : true;
		} else if (id == "telegrambot_sendimagewhendetectchange" || id == "telegrambot_sendimageafterdetectigchange"){
			config.telegramConfig.sendImageWhenDetectChange = value == "1";
		} else if (id == "telegrambot_sendgifwhendetectchange" || id == "telegrambot_sendgifafterchange") {
			config.telegramConfig.sendGifWhenDetectChange = value == "1";
		} else if (id == "telegrambot_sendtextwhendetectchange" || id == "telegrambot_sendtextafterchange") {
			config.telegramConfig.sendTextWhenDetectChange = value == "1";
		} else if (id == "telegrambot_sendvideowhendetectchange" || id == "telegrambot_sendvideoafterchange") {
			config.telegramConfig.sendVideoWhenDetectChange = value == "1";
		} else if (id == "usetelegrambot" || id == "activatetelegrambot") {
			Utils::toLowerCase(value);
			config.telegramConfig.useTelegramBot = value == "0" ? false : true;
		} else if (id == "sendimageofallcameras" || id == "sendImageallcameras") {
			Utils::toLowerCase(value);
			config.sendImageOfAllCameras = value == "0" ? false : true;
		} else if(id == "authuserstosendactions" || id == "authserssendactions"){
			config.authUsersToSendActions = std::move(Utils::SplitString(value, ","));
		} else if (id == "ratioscaleoutput") {
			try {
				config.ratioScaleOutput = std::stod(value);
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "gifresizepercentage" || id == "gifresize") {
			try {
				ulong val = std::stoi(value);
				
				if (val >= 0 && val <= 100) {
					config.gifResizePercentage = val;
				} else {
					sucess = false;
				}
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "gifframes") {
			std::vector<std::string> results = Utils::GetRange(value);
			size_t sz = results.size();
			if (sz == 3) {
				try {
					config.numberGifFrames.framesBefore = size_t(std::stol(results[0]));
					config.numberGifFrames.framesAfter = size_t(std::stol(results[2]));
				} catch (std::invalid_argument e) {
					sucess = false;
				}
			} else {
				sucess = false;
			}
		} else if (id == "showignoredareas") {
			Utils::toLowerCase(value);
			config.showIgnoredAreas = value == "0" ? false : true;
		} else if (id == "detectionmethod") {
			try {
				config.detectionMethod = (DetectionMethod)std::stoi(value);
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "imagesfolder" || id == "mediafolder") {
			config.imagesFolder = value;
		} else if (id == "uselocalnotifications") {
			config.localNotificationsConfig.useLocalNotifications = value == "1";
		} else if (id == "localnotifications_sendtextwhendetectchange" || id == "localnotification_sendtextwhendetectchange") {
			config.localNotificationsConfig.sendTextWhenDetectChange = value == "1";
		} else if (id == "localnotifications_sendimagewhendetectchange" || id == "localnotification_sendimagewhendetectchange") {
			config.localNotificationsConfig.sendImageWhenDetectChange = value == "1";
		} else if (id == "localnotifications_sendgifwhendetectchange" || id == "localnotification_sendgifwhendetectchange") {
			config.localNotificationsConfig.sendGifWhenDetectChange = value == "1";
		} else if (id == "localnotifications_sendvideowhendetectchange" || id == "localnotification_sendvideowhendetectchange") {
			config.localNotificationsConfig.sendVideoWhenDetectChange = value == "1";
		} else if (id == "analizebeforeafterchangeframes") {
			config.analizeBeforeAfterChangeFrames = value == "1";
		} else if (id == "framestoanalyzechangevalidity") {
			std::vector<std::string> results = Utils::GetRange(value);
			size_t sz = results.size();
			if (sz == 3) {
				try {
					config.framesToAnalyzeChangeValidity.framesBefore = size_t(std::stol(results[0]));
					config.framesToAnalyzeChangeValidity.framesAfter = size_t(std::stol(results[2]));
				} catch (std::invalid_argument e) {
					sucess = false;
				}
			}
		} else if (id == "savechangeinvideo") {
			config.saveChangeInVideo = value == "1";
		} else if (id == "drawchangefoundbetweenframes") {
			config.drawChangeFoundBetweenFrames = value == "1";
		}  else if (id == "messageontextnotification") {
			config.messageOnTextNotification = value;
		}  else if (id == "drawtraceofchangefoundon") {
			try {
				int val = std::stoi(value);
				config.drawTraceOfChangeFoundOn = (DrawTraceOn)val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else {
			sucess = false;
		}
		
		return sucess;
	}

	/// <summary> Saves the given id and value into the corresponding member of the camera config </summary>
	bool SaveIdVal(CameraConfiguration& config, std::string id, std::string value) {
		bool sucess = true;
		Utils::toLowerCase(id);

		if (id == "cameraname" || id == "name") {
			config.cameraName = value;
		} else if (id == "order") {
			try {
				int val = std::stoi(value);
				config.order = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "url") {
			config.url = value;
		} else if (id == "rotation") {
			try {
				int val = std::stoi(value);
				config.rotation = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "type") {
			try {
				int val = std::stoi(value);
				config.type = (CAMERATYPE)val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "hitthreshold") {
			std::replace(value.begin(), value.end(), ',', '.');
			try {
				float val = std::stof(value);
				config.hitThreshold = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "roi") {
			std::vector<int> results = Utils::GetNumbersString(value);
			if (results.size() == 4) {
				config.roi = cv::Rect(cv::Point(results[0], results[1]), cv::Size(results[2], results[3]));
			} else {
				sucess = false;
			}
		} else if (id == "thresholdnoise" || id == "noisethreshold") {
			std::replace(value.begin(), value.end(), ',', '.');
			try {
				double val = std::stof(value);
				config.noiseThreshold = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "secondswaitentryexit") {
			try {
				int val = std::stoi(value);
				config.secondsWaitEntryExit = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} 
		else if (id == "minimumthreshold") {
			try {
				int val = std::stoi(value);
				config.minimumThreshold = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "increasetresholdfactor" || id == "increaseTreshold") {
			std::replace(value.begin(), value.end(), ',', '.');
			try {
				double val = std::stof(value);
				config.increaseTresholdFactor = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "updatethresholdfrequency") {
			try {
				uint32_t val = std::stoi(value);
				config.updateThresholdFrequency = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "usehighconstrast") {
			config.useHighConstrast = value == "1";
		} else if (id == "ignoredareas"){
			std::vector<int> results = Utils::GetNumbersString(value);
			
			if (results.size() % 4 == 0) {
				for (size_t i = 0; i < results.size() / 4; i++) {
					int base = i * 4;
					config.ignoredAreas.push_back(cv::Rect(cv::Point(results[base], results[base+1]), cv::Size(results[base+2], results[base+3])));			
				}
			} else {
				sucess = false;
			}
		} else if (id == "framestoanalyze") {
			std::vector<std::string> results = Utils::GetRange(value);
			size_t sz = results.size();
			if (sz >= 1) {
				try {
					if (results[0] == "..") {
						if (sz >= 2)
							config.framesToAnalyze.framesAfter = size_t(std::stol(results[1]));
					} else {
						config.framesToAnalyze.framesBefore = size_t(std::stol(results[0]));

						if (sz >= 3)
							config.framesToAnalyze.framesAfter = size_t(std::stol(results[2]));
					}
				} catch (std::invalid_argument e) {
					sucess = false;
				}
			} else {
				sucess = false;
			}
		} else if (id == "thresholdfindingsonignoredarea") {
			try {
				int val = std::stoi(value);
				config.thresholdFindingsOnIgnoredArea = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "minpercentageareaneededtoignore") {
			try {
				double val = std::stod(value);
				config.minPercentageAreaNeededToIgnore = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "pointsdiscriminators" || id == "pointsdiscriminatorarea") {
			sucess = Utils::String2DiscriminatorArea(value, config.pointDiscriminators);
		} else if (id == "minpercentageinsideallowdiscriminator") {
			try {
				double val = std::stod(value);
				config.minPercentageInsideAllowDiscriminator = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else if (id == "maxpercentageinsidedenydiscriminator") {
			try {
				double val = std::stod(value);
				config.maxPercentageInsideDenyDiscriminator = val;
			} catch (std::invalid_argument e) {
				sucess = false;
			}
		} else {
			sucess = false;
		}
		
		return sucess;
	}
}