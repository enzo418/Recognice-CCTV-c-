#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include <sys/stat.h> // To check if file exist
#include <unordered_map> // To get a unique file id from a camera url

#include "ImageManipulation.hpp"
#include "utils.hpp"
#include "ConfigurationFile.hpp"
#include "Configuration.hpp"

#ifdef WINDOWS
#include <Windows.h>
#include <CommCtrl.h>
#include "../windows/NotificationIconWindows.hpp"
#else
#include "../unix/NotificationIconUnix.hpp"
#define sprintf_s sprintf
#endif

#define RESIZERESOLUTION cv::Size(RES_WIDTH, RES_HEIGHT)
#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std::chrono;

void SaveAndUploadImage(MessageArray& messages, ProgramConfig& programConfig, bool& stop) {
    cv::Mat frame;
    std::string date;

    while (!stop) {
        size_t size = messages.size();
        for (size_t i = 0; i < size; i++) {
            // take the first message and delete it
            //Message msg = messages[i];
            
            std::cout << "Sending message => " 
                << "IsText=" << (messages[i].IsText() ? "yes" : "no") 
                << "\tIsSound=" << (messages[i].IsSound() ? "yes" : "no")
                << "\tFrame size=" << messages[i].image.cols << "," << messages[i].image.rows
                << "\tText=" << messages[i].text
                << std::endl;

            std::string command;

            if (!messages[i].IsText() && !messages[i].IsSound() && messages[i].image.rows > 0) {
                std::string filename = "img_";
                filename += messages[i].text;
                filename += ".jpg";
                //std::cout << "Filename = " << filename << std::endl;
                cv::imwrite("./saved_imgs/" + filename, messages[i].image);

                #ifdef WINDOWS
                command = "curl -F \"chat_id=" + programConfig.telegramConfig.chatId + "\" -F \"photo=@saved_imgs\\" + filename + "\" \\ https://api.telegram.org/bot" + programConfig.telegramConfig.apiKey + "/sendphoto";                
                #else
                command = "curl -F chat_id=" + programConfig.telegramConfig.chatId + " -F photo=\"@saved_imgs//"+ filename + "\" https://api.telegram.org/bot" + programConfig.telegramConfig.apiKey + "/sendphoto";
                #endif

                std::cout << "command => " << command << std::endl;
                system(command.c_str());
            } else if(messages[i].IsText()) {
                command = "curl -F chat_id=" + programConfig.telegramConfig.chatId + " -F text=\"" + messages[i].text + "\" https://api.telegram.org/bot" + programConfig.telegramConfig.apiKey + "/sendMessage";
                system(command.c_str());
            } else {
                PlayNotificationSound();
            }
        }

        // this clears all the messages, wich is not good but
        // messages is bugged when try to copy the message to a new variable...
        messages.clear();

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

/// <summary> Takes a frame from each camera then stack and display them in a window </summary>
void ShowFrames(CameraConfig* configs, const int amountCameras, 
                ProgramConfig& programConfig, bool& stop, HWND hwndl = nullptr, HMODULE g_hinst = nullptr) {
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
            if (configs[i].frames.size() > 0) {
                // if the vector pos i has no frame
                if (!ready[configs[i].order]) {
                    // take the first frame and delete it
                    frames[configs[i].order] = configs[i].frames[0];

                    if (showAreaCameraSees && !showProcessedFrames) {
                        cv::Scalar color = cv::Scalar(255, 0, 0);
                        cv::rectangle(frames[configs[i].order], configs[i].roi.point1, configs[i].roi.point2, color);
                    }

                    configs[i].frames.erase(configs[i].frames.begin());
                    
                    ready[configs[i].order] = true;
                    
                    size++;
                }
            }

            // Check camera state
            if (configs[i].state == NI_STATE_DETECTED) {
                allCamerasInSentry = false;

                // Send notification if the current state != to what the camera state is
                if (currentState != NI_STATE_DETECTED) {

                    // change the current state
                    currentState = NI_STATE_DETECTED;

                    // send the notification
                    ChangeNotificationState(currentState, configs[i].cameraName.c_str(), notificationTitle, hwndl, g_hinst);

                    std::cout << "[W] " << configs[i].cameraName << " detected a person..." << std::endl;
                }
            } else if (configs[i].state == NI_STATE_DETECTING) {
                allCamerasInSentry = false;
                
                // Send notification if the current state != to what the camera state is
                if (currentState != NI_STATE_DETECTING && currentState != NI_STATE_DETECTED) {
                    
                    // change the current state
                    currentState = NI_STATE_DETECTING;

                    // send the notification
                    //ChangeNotificationState(currentState, configs[i].cameraName.c_str(), notificationTitle, hwndl, g_hinst);

                    std::cout << "[I] " << configs[i].cameraName << " is trying to match a person in the frame..." << std::endl;
                }
            } else if (configs[i].state == NI_STATE_SENTRY) {
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

            cv::imshow("TEST", res);
            if (cv::waitKey(1) >= 0) {
                //stop = true;
                //break;
				
                /* Uncomment to use video recorder
                out.release();
                */
            }
        }

        /* Start video recorder support */                
        /*auto now = high_resolution_clock::now();
        auto time = (now - timeLastCheck) / std::chrono::milliseconds(1);
        if (time > 60 * 1000) {
            if (somethingDetected) {
                videosSaved += 1;
                somethingDetected = false;
            } else {
                out.release();
                
                // create a new video
                out.open(GetTimeFormated()+"__"+std::to_string(videosSaved)+".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15., frameSize, true);
                if (!out.isOpened()) {
                    cerr << "Could not open the output video file for write\n";
                }
            }
        } else if (videosSaved == 0) {
            out.open(GetTimeFormated() + "__0.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15., frameSize, true);
            if (!out.isOpened()) {
                cerr << "Could not open the output video file for write\n";
            }
        } */       
        /* End video recorder support */

        std::this_thread::sleep_for(std::chrono::milliseconds(int(interval * 0.7)));
    }
}

///<param name='interval'>Minimum distance (in ms) between frame</param>
void ReadFramesWithIntervals(CameraConfig* config, bool& stop, bool& somethingDetected, 
                             ProgramConfig& programConfig, cv::HOGDescriptor& hog, MessageArray& messages) {
    #pragma region SetupVideoCapture

    // ==============
    //  consts/vars
    // ==============

    // if framesLeft > 0 then do a classifcation over the current frame
    ushort framesLeft = 0; 

    const ushort interval = programConfig.msBetweenFrame;
    
    // higher interval -> lower max & lower interval -> higher max
    const ushort maxFramesLeft = (100.0 / (interval+0.0)) * 70; // 100 ms => max = 70 frames

    const int x = config->roi.point1.x;
    const int h = abs(config->roi.point2.y - config->roi.point1.y);
    const int w = abs(config->roi.point2.x - config->roi.point1.x);
    const ulong frameArea = w * h;

    const char* camName = &config->cameraName[0];
    const CAMERATYPE camType = config->type;
    const int changeThreshld = config->changeThreshold;

    const bool showPreview = programConfig.showPreview;   
    const bool showProcessedImages = programConfig.showProcessedFrames;
    const bool sendImageAfterChange = programConfig.sendImageWhenDetectChange;
        
    int totalNonZeroPixels = 0;
    
    // ammount of frame that recognition will be active before going to idle state    
    const uint8_t framesToRecognice = (100 / (interval + 0.0)) * 30; 

    // used to store the current frame to be able to compare it to the next one
    cv::Mat lastFrame;
    
    // used to store the diff frame
    cv::Mat diff;

    cv::VideoCapture capture(config->url);

    std::cout << "Opening " << camName << "..." << std::endl;
    assert(capture.isOpened());

    cv::Mat frame;
    cv::Mat invalid;
    cv::Mat frameToShow;

#pragma endregion

    auto timeLastframe = high_resolution_clock::now();
    bool newFrame = false;

    // ----------------
    //  Save&Upl image
    // ----------------
    auto timeLastSavedImage = high_resolution_clock::now();
    
    // -----------
    //  Messages
    // -----------
    auto lastMessageSended = high_resolution_clock::now();
    const int secondsBetweenMessages = programConfig.secondsBetweenMessage;

    while (!stop && capture.isOpened()) {
        const NISTATE camState = config->state;

        auto now = high_resolution_clock::now();
        auto intervalFrames = (now - timeLastframe) / std::chrono::milliseconds(1);
        if (intervalFrames >= interval) {
            capture.read(frame);
            timeLastframe = high_resolution_clock::now();
            newFrame = true;
        } else {
            capture.read(invalid); // keep reading to avoid error on VC.
        }

        if (newFrame) {
#pragma region AnalizeFrame

            //assert(frame.rows != 0); // check if the frame is valid
            if (frame.rows == 0) {
                //std::cout << "Received a invalid frame from \"" << camName << "\". Restarting connection..." << std::endl;

                capture.release();
                capture.open(config->url);

                //framesCount--;
                assert(capture.isOpened());
                //std::cout << "Connected to \"" << camName << "\" successfully...." << std::endl;
                continue;
            }

            cv::resize(frame, frame, RESIZERESOLUTION);

            //if (showPreview)
                frameToShow = frame.clone();

            // Take the region of interes
            if (!config->roi.isEmpty()) {
                cv::Rect roi(config->roi.point1, config->roi.point2);
                frame = frame(roi);
            }

            // Then rotate it
            if (config->rotation != 0) ImageManipulation::RotateImage(frame, config->rotation);

            cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

            if (lastFrame.rows > 0) {
                cv::absdiff(lastFrame, frame, diff);

                // 45 works perfect with most of the cameras/resolution
                cv::threshold(diff, diff, config->noiseThreshold, 255, cv::THRESH_BINARY);

                totalNonZeroPixels = cv::countNonZero(diff);

                if (showProcessedImages && showPreview) {
                    // place diff image on top the frame img
                    cv::addWeighted(frame, 1, diff, 8, 12, diff);
					
                    config->frames.push_back(diff);
                }
            }

            if (totalNonZeroPixels > changeThreshld * 0.7) {
                std::cout << config->cameraName
                    << " Non zero pixels=" << totalNonZeroPixels
                    << " Configured threshold=" << changeThreshld
                    << std::endl;
            }
            lastFrame = frame;

            if (totalNonZeroPixels > changeThreshld) {
                if (camState != NI_STATE_DETECTED
                    && camType != CAMERA_SENTRY
                    && camState != NI_STATE_DETECTING)
                    config->state = NI_STATE_DETECTING; // Change the state of the camera
                                
                // every secondsBetweenMessages send a message to the telegram bot
                intervalFrames = (now - lastMessageSended) / std::chrono::seconds(1);
                if (intervalFrames >= secondsBetweenMessages) {
                    // Play a sound
                    messages.push_back(Message());

                    if(sendImageAfterChange){                        
                        // and then send a message
                        const char* txt = "temp_image";
                        Message msg = Message(frameToShow, txt);                        
                        messages.push_back(msg);
                    }else{
                        // and then send a message
                        char msg[100];
                        snprintf(msg, MESSAGE_SIZE, "[W] Motion detected in %s", config->cameraName.c_str());
                        messages.push_back(Message(msg));
                    }

                    lastMessageSended = high_resolution_clock::now();
                }

                // Increment frames left
                if (framesLeft < maxFramesLeft)
                    framesLeft += framesToRecognice;
            }

            if (camType == CAMERA_SENTRY) {
                if (framesLeft > 0) {
                    if (camState != NI_STATE_DETECTING) {
                        config->state = NI_STATE_DETECTING;
                    }
                    framesLeft--;
                } else {
                    config->state = NI_STATE_SENTRY;
                }

                newFrame = false;
                if (showPreview && !showProcessedImages)
                    config->frames.push_back(frameToShow);
            } else {
                std::vector<cv::Rect> detections;
                std::vector< double > foundWeights;
                if (RECOGNICEALWAYS || framesLeft > 0) {
                    hog.detectMultiScale(frame, detections, foundWeights, config->hitThreshold, cv::Size(8, 8), cv::Size(4, 4), 1.05);
                    for (size_t i = 0; i < detections.size(); i++) {
                        detections[i].x += x;
                        cv::Scalar color = cv::Scalar(0, foundWeights[i] * foundWeights[i] * 200, 0);
                        cv::rectangle(frameToShow, detections[i], color);
                    }

                    now = high_resolution_clock::now();
                    auto time = (now - timeLastSavedImage) / std::chrono::seconds(1);
                    if (detections.size() > 0 && time >= programConfig.secondsBetweenImage) {
                        config->state = NI_STATE_DETECTED;
                        somethingDetected = true;
                        Message msg = Message(frameToShow, "");
                        snprintf(msg.text, MESSAGE_SIZE, "%s", Utils::GetTimeFormated().c_str());
                        messages.push_back(msg);
                        std::cout << "Pushed image seconds=" << time << " of " << programConfig.secondsBetweenImage << std::endl;
                        timeLastSavedImage = high_resolution_clock::now();
                    }

                    framesLeft--;
                    //std::cout << camName << " -- Frames " << framesLeft << std::endl;
                }
#pragma endregion

                if (framesLeft == 0) {
                    config->state = NI_STATE_SENTRY;
                }

                newFrame = false;

                if (showPreview && !showProcessedImages)
                    config->frames.push_back(std::move(frameToShow));
            }
        }
    }

    std::cout << "Closed connection with " << camName << std::endl;

    capture.release();
}


int StartDetection(std::vector<CameraConfig>& configs, ProgramConfig& programConfig, 
                   HWND hwnd, HMODULE g_hInst) {
    bool stop = false;
    bool somethingDetected = false;

    std::vector<std::thread> threads;

    // Array of messages that the cameras order to send to telegram.
    MessageArray messages;

    int configsSize = configs.size();

    if (configsSize == 0) {
        std::cout << "Couldn't fin cameras in the config file." << configsSize << std::endl;
        std::getchar();
        return 0;
    }

    Utils::FixOrderCameras(configs);

    std::cout << "Cameras found: " << configsSize << std::endl;

    // start hog decriptor
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    // Start a thread for each camera
    for (size_t i = 0; i < configsSize; i++) {
        threads.push_back(std::thread(ReadFramesWithIntervals, &configs[i], std::ref(stop), std::ref(somethingDetected), std::ref(programConfig), std::ref(hog), std::ref(messages)));
    }

    if (programConfig.showPreview) {
        // Start the thread to show the images captured.
        threads.push_back(std::thread(ShowFrames, &configs[0], configsSize, std::ref(programConfig), std::ref(stop), hwnd, g_hInst));
    }

    // Start a thread for save and upload the images captured    
    threads.push_back(std::thread(SaveAndUploadImage, std::ref(messages), std::ref(programConfig), std::ref(stop)));

    /*if (!programConfig.showPreview) {
        std::cout << "Press a key to stop the program.\n";
        std::getchar();
        stop = true;
    }*/
	
	std::cout << "Press a key to stop the program.\n";
	std::getchar();
	
	stop = true;

    // Wait until every thread finish
    int szThreads = threads.size();
    for (int i = 0; i < szThreads; i++) {
        if (i == szThreads - 1)
            std::cout << "Please wait... 3 seconds until release all the resources used." << std::endl;
        threads[i].join();
    }
}

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(int argc, char* argv[]){
    bool isUrl = false;
    bool startConfiguration = false;
    char url[200];

    // read for url... this is used for the configurator program.
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-u") == 0) {
                isUrl = true;
            } else if (strcmp(argv[i], "--config") == 0) {
                startConfiguration = true;
            } else if (strlen(argv[i]) > 1 && isUrl) {
                strcpy_s(url, argv[i]);
            }
        }

        if (isUrl) {
            std::hash<std::string> hasher;
            size_t name = hasher(url);
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
    }
	

    // configs
    std::vector<CameraConfig> camerasConfigs;
    ProgramConfig programConfig;

    // Get the cameras and program configurations
    Config::File::ConfigFileHelper fhelper;
    fhelper.ReadFile(programConfig, camerasConfigs);

    if (camerasConfigs.size() == 0 || startConfiguration)
        Config::StartConfiguration(camerasConfigs, programConfig);

#ifdef WINDOWS
	HWND hwnd = GetConsoleWindow();
	HMODULE g_inst = GetModuleHandleA(NULL);
	AddNotificationIcon(hwnd, g_inst);
	StartDetection(camerasConfigs, programConfig, hwnd, g_inst);
	DeleteNotificationIcon();
#else
	StartDetection(camerasConfigs, programConfig, nullptr, nullptr);
#endif
}