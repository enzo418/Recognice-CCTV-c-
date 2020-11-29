#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <thread>
#include <map>
#include <chrono>
#include <ctime>
#include <sys/stat.h> // To check if file exist
#include <unordered_map> // To get a unique file id from a camera url
#include <signal.h> // to catch ctrl c signal

#include "ImageManipulation.hpp"
#include "utils.hpp"
#include "ConfigurationFile.hpp"

#include "TelegramBot.hpp"

#ifdef WINDOWS
#else
#define sprintf_s sprintf
#endif
#include "Camera.hpp"

#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std::chrono;

class Recognize {
private:
	size_t indexMainThreadCameras;

	std::vector<Camera> cameras;

	ProgramConfiguration programConfig;
	CamerasConfigurations camerasConfigs;

	cv::HOGDescriptor hogDescriptor;
public:
	bool stop = false;
	bool close = false;

	std::vector<std::thread> threads;

	Recognize(void);

	void Start(Configurations configs, bool startPreviewThread, bool startActionsThread);

	void StartActionsBot();
	void StartNotificationsSender();
	void StartPreviewCameras();
	void StartCamerasThreads();
};

Recognize::Recognize() {
	this->hogDescriptor.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
}

void Recognize::Start(Configurations configs, bool startPreviewThread, bool startActionsThread) {	
	this->programConfig = configs.programConfig;
	this->camerasConfigs = configs.camerasConfigs;
	
	this->indexMainThreadCameras = this->threads.size();
	std::cout << "pushed thread of cameras in " << this->threads.size() << std::endl;
	this->threads.push_back(std::thread(&Recognize::StartCamerasThreads, this));

	if (startPreviewThread) {
		std::cout << "pushed thread of preview in " << this->threads.size() << std::endl;
		// Start the thread to show the images captured.
		this->threads.push_back(std::thread(&Recognize::StartPreviewCameras, this));
	}

	if (startActionsThread) {
		std::cout << "pushed thread of actions bot in " << this->threads.size() << std::endl;
		this->threads.push_back(std::thread(&Recognize::StartActionsBot, this));
	}

	if (programConfig.telegramConfig.useTelegramBot) {
		std::cout << "pushed thread of notifications in " << this->threads.size() << std::endl;		
		// Start a thread for save and upload the images captured    
		this->threads.push_back(std::thread(&Recognize::StartNotificationsSender, this));
	}
}

void Recognize::StartCamerasThreads() {
	bool somethingDetected = false;

	Utils::FixOrderCameras(this->camerasConfigs);
	
	// Create the cameras objs
	for (auto &config : this->camerasConfigs) {
		this->cameras.push_back(Camera(config, &programConfig, &this->stop, &this->hogDescriptor));
	}

	// Start a thread for each camera
	for (size_t i = 0; i < this->cameras.size(); i++) {
		this->threads.push_back(this->cameras[i].StartDetection());
	}
}

void Recognize::StartActionsBot() {
	auto lastCheck = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	std::string lastMessage = "";
	std::string message = "";
	std::time_t unix_time_start = std::time(0);
	
	while (!close) {
		now = std::chrono::high_resolution_clock::now();
		auto diff = (now - lastCheck) / std::chrono::seconds(1);
		if(diff >= 9) {
			std::time_t unix_time = 0;
			std::string fromId = TelegramBot::GetLastMessageFromBot(programConfig.telegramConfig.apiKey, message, unix_time, programConfig.authUsersToSendActions);
			std::string reply = "Comando no reconocido.";

			if(unix_time > unix_time_start) {
				std::cout << "Message: " << message << " Last Message: " << lastMessage << std::endl;
				
				if(lastMessage != message){
					/// TODO: Read the messages needed from a file so is user-specific.
					if (message == "/apagar") {
						close = true; // closes the bot
						stop = true; // closes the cameras connections

						reply = "Reconocedor apagado.";
					} else if (message == "/pausar"){
						stop = true;

						if(this->threads[this->indexMainThreadCameras].joinable())
							this->threads[this->indexMainThreadCameras].join();

						cv::destroyAllWindows();

						reply = "Reconocedor pausado.";
					} else if (message == "/reanudar") {
						stop = false;
						this->threads[this->indexMainThreadCameras] = std::thread(&Recognize::StartCamerasThreads, this); 

						reply = "Reconocedor reanudado.";
					}

					TelegramBot::SendMessageToChat(reply, fromId, programConfig.telegramConfig.apiKey);
					lastMessage = message;
				}
			}

			lastCheck = std::chrono::high_resolution_clock::now();
		}

		std::this_thread::sleep_for(std::chrono::seconds(3));		
	}	
}

void Recognize::StartPreviewCameras() {
    // ============
	//  Variables 
	// ============
	  
	size_t amountCameras = this->camerasConfigs.size();

	// saves the frames to show in a iteration
	std::vector<cv::Mat> frames;

	const ushort interval = programConfig.msBetweenFrame;
	
	// saves the cameras that are ready to be displayed
	std::vector<bool> ready;

	// Windows notification alert message and title
	//const char* notificationMsg = "";
	const char* notificationTitle = "Camera alert! Someone Detected";

	const bool showAreaCameraSees = programConfig.showAreaCameraSees;
	const bool showProcessedFrames = programConfig.showProcessedFrames;
	
	// counts the cameras displayed
	uint8_t size = 0; 

	uint8_t stackHSize = amountCameras > 1 ? 2 : 1;

	// resolution of each frame
	cv::Mat res;

	// saves the current state of the notification icon
	NISTATE currentState = NI_STATE_SENTRY;

	// used to, in case is false, display the last frame of the camera that has no frame
	// in the moment of the iteration.
	bool isFirstIteration = true;
	frames.resize(amountCameras);

	// init vector of cameras ready
	for (size_t i = 0; i < amountCameras; i++) {
		ready.push_back(false);
	}

	ready.shrink_to_fit();
	
	/* Video writer */
	/* Uncomment to use video recorder
	cv::VideoWriter out;
	ushort videosSaved = 0;
	auto timeLastCheck = high_resolution_clock::now();
	cv::Size frameSize;
	*/

	/* Image saver */
	auto timeLastSavedImage = high_resolution_clock::now();
	ushort secondsBetweenImage = 2;

	// ============
	//  Main loop 
	// ============

	while (!stop) {
		if (this->camerasConfigs.size() != amountCameras) {
			std::cout << "started resizing: last="<< amountCameras << " new=" << this->camerasConfigs.size() << std::endl;
			amountCameras = this->camerasConfigs.size();
			stackHSize = amountCameras > 1 ? 2 : 1;
			frames.resize(amountCameras);
			ready.resize(amountCameras);

			// init vector of cameras ready
			for (size_t i = 0; i < amountCameras; i++) {
				ready.push_back(false);
			}

			ready.shrink_to_fit();
			std::cout << "resized all." << std::endl;
		}

		if (this->cameras.size() > 0) {
			// if all cameras are in sentry state
			bool allCamerasInSentry = true;

			for (size_t i = 0; i < amountCameras; i++) {
				if (cameras[i].frames.size() > 0) {
					// if the vector pos i has no frame
					if (!ready[cameras[i].config.order]) {
						// take the first frame and delete it
						frames[cameras[i].config.order] = cameras[i].frames[0];

						if (showAreaCameraSees && !showProcessedFrames) {
							cv::Scalar color = cv::Scalar(255, 0, 0);
							cv::rectangle(frames[cameras[i].config.order], cameras[i].config.roi.point1, cameras[i].config.roi.point2, color);
						}

						cameras[i].frames.erase(cameras[i].frames.begin());
						
						ready[cameras[i].config.order] = true;
						
						size++;
					}
				}

				// Check camera state
				if (cameras[i].config.state == NI_STATE_DETECTED) {
					allCamerasInSentry = false;

					// Send notification if the current state != to what the camera state is
					if (currentState != NI_STATE_DETECTED) {

						// change the current state
						currentState = NI_STATE_DETECTED;

						// send the notification
						//ChangeNotificationState(currentState, cameras[i].config.cameraName.c_str(), notificationTitle);

						std::cout << "[W] " << cameras[i].config.cameraName << " detected a person..." << std::endl;
					}
				} else if (cameras[i].config.state == NI_STATE_DETECTING) {
					allCamerasInSentry = false;
					
					// Send notification if the current state != to what the camera state is
					if (currentState != NI_STATE_DETECTING && currentState != NI_STATE_DETECTED) {
						
						// change the current state
						currentState = NI_STATE_DETECTING;

						// send the notification
						//ChangeNotificationState(currentState, configs[i].cameraName.c_str(), notificationTitle, hwndl, g_hinst);

						std::cout << "[I] " << cameras[i].config.cameraName << " is trying to match a person in the frame..." << std::endl;
					}
				} else if (cameras[i].config.state == NI_STATE_SENTRY) {
					allCamerasInSentry = allCamerasInSentry && true;
				}     
			}

			if (allCamerasInSentry && currentState != NI_STATE_SENTRY) {
				currentState = NI_STATE_SENTRY;

				//ChangeNotificationState(currentState, "Sentry camera has something.", notificationTitle, hwndl, g_hinst);

				std::cout << "[I]" << " All the cameras are back to sentry mode." << std::endl;
			}
			
			if (size == amountCameras || !isFirstIteration) {
				res = ImageManipulation::StackImages(&frames[0], amountCameras, stackHSize);
				size = 0;
  
				isFirstIteration = false;

				/* Uncomment if want to record video
				if (frameSize.width != res.cols) {
					frameSize.width = res.cols;
					frameSize.height = res.rows;
				}
				*/

				for (size_t i = 0; i < amountCameras; i++)
					ready[i] = false;  

				double scaleW = programConfig.outputResolution.width * programConfig.ratioScaleOutput;
				double scaleH = programConfig.outputResolution.height * programConfig.ratioScaleOutput;
				if (!programConfig.outputResolution.empty()){
					scaleW = programConfig.outputResolution.width * programConfig.ratioScaleOutput;
					scaleH = programConfig.outputResolution.height * programConfig.ratioScaleOutput;
				} else {
					scaleW = res.cols * programConfig.ratioScaleOutput;
					scaleH = res.rows * programConfig.ratioScaleOutput;
				}

				cv::resize(res, res, cv::Size(scaleW, scaleH));

				cv::imshow("Cameras", res);
				cv::waitKey(1);            
				programConfig.frameWithAllTheCameras = std::move(res);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(int(interval * 0.7)));
	}  
}

void Recognize::StartNotificationsSender() {
	cv::Mat frame;
	std::string date;

	while (!stop) {
		for (auto &&camera : cameras) {
			size_t size = camera.pendingNotifications.size();
			for (size_t i = 0; i < size; i++) {	
				std::cout << "Sending notification of type " << camera.pendingNotifications[i].type << std::endl;

				camera.pendingNotifications[i].send(programConfig);
			}
			
			// Send gif
			if (programConfig.useGifInsteadImage && camera.gifFrames.sendGif) {
				// -----------------------------------------------------------
				// Take before and after frames and combine them into a .gif
				// -----------------------------------------------------------
				const std::string identifier = std::to_string(clock());
				const std::string imageFolder = programConfig.imagesFolder;
				const std::string root = "./" + imageFolder + "/" + identifier;
				const std::string gifPath = root + ".gif";
				const size_t gframes = programConfig.halfGifFrames;
				std::string location;

				cv::Point2f vertices[4];
				camera.gifFrames.rotatedRectChange.points(vertices);

				// before: 
				size_t totalFrames = 0;
				for (size_t i = camera.gifFrames.indexBefore;;) {
					if (totalFrames < gframes) {						
						location = root + "_" + std::to_string((int)totalFrames) + ".jpg";

						for (int j = 0; j < 4; j++)
							cv::line(camera.gifFrames.before[i], vertices[j], vertices[(j+1)%4], cv::Scalar(255,255,170), 2);

						cv::imwrite(location, camera.gifFrames.before[i]);

						totalFrames++;
						i = (i + 1) >= gframes ? 0 : (i + 1);
					} else
						break;
				}
				
				for (; totalFrames < gframes*2; totalFrames++) {
					location = root + "_" + std::to_string((int)totalFrames) + ".jpg";

					for (int j = 0; j < 4; j++)
						cv::line(camera.gifFrames.after[totalFrames - gframes], vertices[j], vertices[(j+1)%4], cv::Scalar(255,255,170), 2);

					cv::imwrite(location, camera.gifFrames.after[totalFrames - gframes]);
				}

				std::string command = "convert -resize " + std::to_string(programConfig.gifResizePercentage) + "% -delay 23 -loop 0 " + root + "_{0.." + std::to_string(gframes*2-1) + "}.jpg " + gifPath;

				std::system(command.c_str());

				TelegramBot::SendMediaToChat(gifPath, "Movimiento detectado.", programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey, true);

				// ---
				// update gif collection data
				// ---
				camera.gifFrames.indexBefore = 0;
				camera.gifFrames.indexAfter = 0;

				camera.gifFrames.sendGif = false;
				camera.gifFrames.updateAfter = false;
				camera.gifFrames.updateBefore = true;				
			}

			// This proc shouldn't clear all the notifcations since it's a multithread process :p
			camera.pendingNotifications.clear();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}
}

void CheckRegisters(CameraConfiguration* configs, const int amountCameras, bool& stop){    
	static auto lastTimeCleaned = high_resolution_clock::now();
	size_t lastCleanedSize = 0;
	const int secondsIntervalClean = 120;
	const int cleanRegistersThreshold = 120;
	const size_t cleanItems = 50;

	while (!stop) {
		auto now = std::chrono::high_resolution_clock::now();

		// run over cameras
		for(size_t i = 0; i < amountCameras; i++){
			// if the cameras has a register
			if(configs[i].registers.size() > 0) 
			{
				// then run over the registers
				for (size_t j = 0; j < configs[i].registers.size(); j++)
				{
					// if we find a register that is not finished
					if(!configs[i].registers[j].finished)
					{
						// again, run over the register to find another that is not finished
						for (size_t k = j+1; k < configs[i].registers.size(); k++)
						{                         
							// if this register is not finished and is different to the register of above
							if(!configs[i].registers[k].finished && configs[i].registers[k].firstPoint != configs[i].registers[j].firstPoint)
							{
								double seconds = configs[i].registers[k].time_point->tm_sec - configs[i].registers[j].time_point->tm_sec;
								// and we are within the time (timeout)
								if(seconds <= configs[i].secondsWaitEntryExit){
									configs[i].registers[k].partner = configs[i].registers[j].id;
									configs[i].registers[j].partner = configs[i].registers[k].id;

									configs[i].registers[k].finished = true;
									configs[i].registers[j].finished = true;

									configs[i].registers[k].lastPoint = configs[i].registers[k].firstPoint;
									configs[i].registers[j].lastPoint = configs[i].registers[k].firstPoint;

									std::cout << "[T] Time k = "; Utils::LogTime(configs[i].registers[k].time_point);
									std::cout << "\n    Time j = "; Utils::LogTime(configs[i].registers[j].time_point);

									std::cout << "\n [I] A person was found in " << configs[i].cameraName << " " 
									<< seconds << " seconds ago  or "
									<< " starting from the " << (configs[i].registers[j].firstPoint == RegisterPoint::entryPoint ? "entry":"exit")
									<< " point and ending at the " << (configs[i].registers[k].firstPoint == RegisterPoint::entryPoint ? "entry":"exit")
									<< " point" << std::endl;
								}
							}
						}                        
					}
				}                  

				// if needed clean the vector of registers
				/// TODO: Insted of just delete them save all of them in a file.
				// if((now - lastTimeCleaned).count() >= secondsIntervalClean && configs[i].registers.size() > cleanRegistersThreshold){
				//     std::vector<size_t> listId;
				//     size_t end = std::min(configs[i].registers.size(), cleanItems);
				//     for (size_t x = 0; x < end; x++) {
				//         if(configs[i].registers[x].finished && !ExistInVector(listId, configs[i].registers[x].partner)){
				//             configs[i].registers.erase(configs[i].registers.begin()+x);
				//             listId.push_back(configs[i].registers[x].id);
				//         }
				//     }

				//     lastTimeCleaned = high_resolution_clock::now();
				// }             
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}    
}

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(int argc, char* argv[]){
	bool isUrl = false;
	bool startConfiguration = false;
	std::string pathConfig = "./config.ini";    

	const std::string keys = 
		"{help h       |            | show help message}"
		"{url          |            | url of the camera to obtain a image}"
		"{config_path  | config.ini | path of the configuration file}"
		"{start_config | no         | start the configuration of the cameras}";

	cv::CommandLineParser parser (argc, argv, keys);

	if (parser.has("help")) {
		parser.printMessage();
		return 0;
	}

	if (parser.has("url")) {
		std::hash<std::string> hasher;
		std::string url = parser.get<std::string>("url");
		size_t name = hasher(url);

		std::cout << "URL => " << url;

		return 0;
		
		char path[75];
		sprintf_s(path, "%lu.jpg", name);

		if (!Utils::FileExist(path)) {
			cv::VideoCapture vc(url);

			cv::Mat frame;

			if (vc.isOpened()) {
				vc.read(frame);
				vc.release();

				cv::resize(frame, frame, RESIZERESOLUTION);

				cv::imwrite(path, frame);                    
			} else {
				return -1;
			}
		}

		std::cout << "image_name=" << path << std::endl;

		return 0;
	}

	if (parser.has("config_path")) {
		pathConfig = parser.get<std::string>("config_path");
	}	

	if(parser.has("start_config")){
		startConfiguration = parser.get<std::string>("start_config") != "no";
	}

	// Get the cameras and program configurations
	Configuration fhelper(pathConfig.c_str());
	fhelper.ReadConfigurations();

	if (fhelper.configurations.camerasConfigs.size() == 0 || startConfiguration){
		if(!startConfiguration)
			std::cout << "Couldn't find the configuration file. \n";        
		fhelper.StartConfiguration();
	}    

	Recognize recognize;

	recognize.Start(std::ref(fhelper.configurations), fhelper.configurations.programConfig.showPreview, fhelper.configurations.programConfig.telegramConfig.useTelegramBot);

	// signal(SIGINT, signal_callback_handler);

	std::cout << "Press a key to stop the program.\n";
	std::getchar();

	recognize.close = true;
	recognize.stop = true;

	for (size_t i = 0; i < recognize.threads.size(); i++) {
		std::cout << "joining thread " << i << std::endl;
		recognize.threads[i].join();
		std::cout << "joined thread " << i << std::endl; 
	}	
}
