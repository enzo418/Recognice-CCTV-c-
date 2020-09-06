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

#include "ImageManipulation.hpp"
#include "utils.hpp"
#include "ConfigurationFile.hpp"

#include "TelegramBot.hpp"

#ifdef WINDOWS
#include <Windows.h>
#include <CommCtrl.h>
#include "../windows/NotificationIconWindows.hpp"
#else
#include "../unix/NotificationIconUnix.hpp"
#define sprintf_s sprintf
#endif

#include "Camera.hpp"

#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std::chrono;

int StartDetection(CamerasConfigurations configs, ProgramConfiguration& programConfig, bool& stop, HWND hwnd, HMODULE g_hInst);

void CheckActionsBot(ProgramConfiguration& programConfig, bool& stop, bool& close, Configuration& fhelper, HWND hwnd, HMODULE g_hInst){
	auto lastCheck = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	std::string lastMessage = "";
	std::string message = "";
	std::time_t unix_time_start = std::time(0);

	std::thread detectionThread = std::thread(StartDetection, 
					std::ref(fhelper.configurations.camerasConfigs), 
					std::ref(fhelper.configurations.programConfig), std::ref(stop), std::ref(hwnd), std::ref(g_hInst));
	
	while (!close) {
		now = std::chrono::high_resolution_clock::now();
		auto diff = (now - lastCheck) / std::chrono::seconds(1);
		if(diff >= 9) {
			std::time_t unix_time = 0;
			std::string fromId = GetLastMessageFromBot(programConfig.telegramConfig.apiKey, message, unix_time /*, authUsersList*/);
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

						if(detectionThread.joinable())
							detectionThread.join();

						cv::destroyAllWindows();

						reply = "Reconocedor pausado.";
					} else if (message == "/reanudar") {
						stop = false;
						detectionThread = std::thread(StartDetection, 
							std::ref(fhelper.configurations.camerasConfigs), 
							std::ref(fhelper.configurations.programConfig), std::ref(stop), std::ref(hwnd), std::ref(g_hInst));

						reply = "Reconocedor reanudado.";
					}

					SendMessageToUser(reply, fromId, programConfig.telegramConfig.apiKey);
					lastMessage = message;
				}
			}

			lastCheck = std::chrono::high_resolution_clock::now();
		}

		std::this_thread::sleep_for(std::chrono::seconds(3));		
	}	

	if(detectionThread.joinable())
		detectionThread.join();

	std::cout << "Wait until the bot closes." << std::endl;
}

void SaveAndUploadImage(std::vector<Camera>& cameras, ProgramConfiguration& programConfig, bool& stop) {
    cv::Mat frame;
    std::string date;

    while (!stop) {
		for (auto &&camera : cameras) {
        	size_t size = camera.pendingAlerts.size();
			for (size_t i = 0; i < size; i++) {
				// take the first message and delete it
				//Message msg = messages[i];
				
				std::cout << "Sending message => " 
					<< "IsText=" << (camera.pendingAlerts[i].IsText() ? "yes" : "no") 
					<< "\tIsSound=" << (camera.pendingAlerts[i].IsSound() ? "yes" : "no")
					<< "\tFrame size=" << camera.pendingAlerts[i].image.cols << "," << camera.pendingAlerts[i].image.rows
					<< "\tText=" << camera.pendingAlerts[i].text
					<< std::endl;

				std::string command;

				if (!camera.pendingAlerts[i].IsText() && !camera.pendingAlerts[i].IsSound() && camera.pendingAlerts[i].image.rows > 0) {
					std::string filename = "img_";
					filename += camera.pendingAlerts[i].text;
					filename += ".jpg";
					//std::cout << "Filename = " << filename << std::endl;
					cv::imwrite("./saved_imgs/" + filename, camera.pendingAlerts[i].image);

					if(programConfig.telegramConfig.useTelegramBot){
						#ifdef WINDOWS						
						Utils::BuildCommand(command, "@saved_imgs\\" + filename, camera.pendingAlerts[i].caption, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);
						#else
						Utils::BuildCommand(command, "@saved_imgs//" + filename, camera.pendingAlerts[i].caption, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);
						#endif
		
						// std::cout << "command => " << command << std::endl;
						system(command.c_str());

						if(programConfig.sendImageOfAllCameras && !programConfig.frameWithAllTheCameras.empty() && cameras.size() > 1){
							std::string filename = "img__all.jpg";
							
							cv::imwrite("./saved_imgs/" + filename, programConfig.frameWithAllTheCameras);							

							#ifdef WINDOWS
							Utils::BuildCommand(command, "@saved_imgs\\" + filename, "Imagen de todas las camaras activas.", programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);
							#else							
							Utils::BuildCommand(command, "@saved_imgs//" + filename, "Imagen de todas las camaras activas.", programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);
							#endif

							// std::cout << "command => " << command << std::endl;
							system(command.c_str());
						}
					}
				} else if(camera.pendingAlerts[i].IsText()) {
					if(programConfig.telegramConfig.useTelegramBot){
						Utils::BuildCommand(command, camera.pendingAlerts[i].text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);						
						system(command.c_str());
					}
				} else {
					PlayNotificationSound();
				}
			}

			// this clears all the camera.pendingAlerts, wich is not good but
			// camera.pendingAlerts is bugged when try to copy the message to a new variable...
			camera.pendingAlerts.clear();
		}

        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
}

void inline ChangeNotificationState(NISTATE state, const char* msg, const char* title,
									HWND hwndl = nullptr, HMODULE g_hinst = nullptr) {
    #ifdef WINDOWS
		int maxTries = 0;
		int count = 0;

		while (!SetStateNotificationIcon(hwndl, g_hinst, state, msg, title) && count <= maxTries) {
			std::cout << "Trying again" << std::endl;
			std::this_thread::sleep_for(chrono::milliseconds(1));        
			count++;
		}

		if (count >= maxTries) {
			std::cout << "Failed to send notif.";
			if (messages) {
				std::cout << " Playing a sound to alert.";
				messages->push_back(Message());
			}
			std::cout << std::endl;
		}
		}
	}
	#endif
}

void CheckRegisters(CameraConfiguration* configs, const int amountCameras, bool& stop){    
    static auto lastTimeCleaned = high_resolution_clock::now();
    size_t lastCleanedSize = 0;
    const int secondsIntervalClean = 120;
    const int cleanRegistersThreshold = 120;
    const size_t cleanItems = 50;

    while (!stop)
    {
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
                //         if(configs[i].registers[x].finished && !findInVector(listId, configs[i].registers[x].partner)){
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


// Used to call functions that need their own thread but doesnt take too much time to complete
// void ParallelMain(CameraConfiguration* configs, const int amountCameras, MessageArray& messages, ProgramConfiguration& programConfig, bool& stop){
    /// TODO: Implement
// }


/// <summary> Takes a frame from each camera then stack and display them in a window </summary>
void ShowFrames(Camera* cameras, const int amountCameras, 
                ProgramConfiguration& programConfig, bool& stop, HWND hwndl = nullptr, HMODULE g_hinst = nullptr) {
    // ============
    //  Variables 
    // ============
      
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

                    /// Only for testing puporses
                        // cv::Scalar colorEntry(0,255,255);
                        // cv::Scalar colorExit(0,255,0);
                        
                        // cv::rectangle(frames[configs[i].order], configs[i].areasDelimiters.rectEntry, colorEntry);                        
                        // cv::rectangle(frames[configs[i].order], configs[i].areasDelimiters.rectExit, colorExit);                        
                    /// ...

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
                    ChangeNotificationState(currentState, cameras[i].config.cameraName.c_str(), notificationTitle, hwndl, g_hinst);

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

            if (!programConfig.outputResolution.empty())
                cv::resize(res, res, cv::Size(programConfig.outputResolution.width, programConfig.outputResolution.height));

            cv::imshow("Cameras", res);
            cv::waitKey(1);            
			programConfig.frameWithAllTheCameras = std::move(res);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(int(interval * 0.7)));
    }
}

int StartDetection(CamerasConfigurations configs, ProgramConfiguration& programConfig, bool& stop,
                   HWND hwnd, HMODULE g_hInst) {
    bool somethingDetected = false;

    std::vector<std::thread> threads;

	std::vector<Camera> cameras;

    // Array of messages that the cameras order to send to telegram.
    MessageArray messages;

    int configsSize = configs.size();

    if (configsSize == 0) {
        std::cout << "Couldn't find cameras in the config file." << std::endl;
        std::getchar();
        return 0;
    }

    Utils::FixOrderCameras(configs);

    std::cout << "Cameras found: " << configsSize << std::endl;	

    // start hog decriptor
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

	// Create the cameras objs
	for (auto &config : configs) {
		cameras.push_back(Camera(config, &programConfig, &stop, &hog));
	}

    // Start a thread for each camera
    for (size_t i = 0; i < configsSize; i++) {
        threads.push_back(cameras[i].StartDetection());
    }

    if (programConfig.showPreview) {
        // Start the thread to show the images captured.
        threads.push_back(std::thread(ShowFrames, &cameras[0], configsSize, std::ref(programConfig), std::ref(stop), hwnd, g_hInst));
    }

    // Start a thread for save and upload the images captured    
    threads.push_back(std::thread(SaveAndUploadImage, std::ref(cameras), std::ref(programConfig), std::ref(stop)));

    // Start a thread to check the registers
    // threads.push_back(std::thread(CheckRegisters, &configs[0], configsSize, std::ref(stop)));

    /*if (!programConfig.showPreview) {
        std::cout << "Press a key to stop the program.\n";
        std::getchar();
        stop = true;
    }*/
	
	// std::cout << "Press a key to stop the program.\n";
	// std::getchar();
	
	// stop = true;

    // Wait until every thread finish
    int szThreads = threads.size();
    for (int i = 0; i < szThreads; i++) {
        if (i == szThreads - 1)
            std::cout << "Please wait... 3 seconds until release all the resources used." << std::endl;
        threads[i].join();
    }

    return 0;
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

	// closes the program when true
	bool close = false; 

	// closes all the cameras when true but keeps updating the last bot message. 
	bool stop = false; 

#ifdef WINDOWS
	HWND hwnd = GetConsoleWindow();
	HMODULE g_inst = GetModuleHandleA(NULL);
	AddNotificationIcon(hwnd, g_inst);

	std::thread bot = std::thread(CheckActionsBot, std::ref(fhelper.configurations.programConfig), std::ref(stop), std::ref(close), fhelper, hwnd, g_inst);

	// StartDetection(camerasConfigs, programConfig, hwnd, g_inst);
	bot.join();

	DeleteNotificationIcon();
#else
	std::thread bot = std::thread(CheckActionsBot, std::ref(fhelper.configurations.programConfig), std::ref(stop), std::ref(close), std::ref(fhelper), nullptr, nullptr);
	
	// StartDetection(fhelper.configurations.camerasConfigs, fhelper.configurations.programConfig, std::ref(stop), nullptr, nullptr);
#endif

        std::cout << "Press a key to stop the program.\n";
	std::getchar();
	
	stop = true;
        close = true;

        bot.join();
}
