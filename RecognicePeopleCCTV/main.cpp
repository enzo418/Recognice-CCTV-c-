#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include "ImageManipulation.h"
#include <Windows.h>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include "API_KEYS.h"
#include <sys/stat.h> // To check if file exist
#include <unordered_map> // To get a unique file id from a camera url
#include "utils.h"
#include "ConfigFileHelper.h"
#include "NotificationIcon.h"
#include <CommCtrl.h>

#define RESIZERESOLUTION cv::Size(RES_WIDTH, RES_HEIGHT)
#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

// forward declarations
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
// ...

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std;
using namespace chrono;

WNDPROC prevWndProc;

void RecordSampleOfCamera(CameraConfig& config) {
    cv::VideoWriter out(config.cameraName + ".avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15., cv::Size(1280, 720), true);
    if (!out.isOpened()) {
        cerr << "Could not open the output video file for write\n";
        throw "Erorr.";
    }

    cv::VideoCapture capture(config.url);

    if (!capture.isOpened()) {
        throw "Error: Couldn't open  camera " + config.cameraName + " rtsp.";
    }

    cv::Mat frame;

    while (capture.isOpened()) {
        if (!capture.read(frame)) {
            //Error
        }
        out.write(frame);

        cv::imshow("TEST", frame);

        if (cv::waitKey(1) >= 0) break;
    }

    out.release();
    capture.release();
}

void SaveAndUploadImage(CameraConfig* configs, const int amountCameras, ProgramConfig& programConfig, bool& stop) {
    cv::Mat frame;
    std::string date;

    while (!stop) {
        for (ushort i = 0; i < amountCameras; i++) {
            if (configs[i].framesToUpload.size() > 0) {
                // take the first frame and delete it
                frame = std::get<0>(configs[i].framesToUpload[0]);
                date = std::get<1>(configs[i].framesToUpload[0]);

                configs[i].framesToUpload.erase(configs[i].framesToUpload.begin());

                std::string filename = "img_" + date + ".jpg";
                cv::imwrite("saved_imgs/" + filename, frame);

                const std::string ce = "curl -F \"chat_id=" + programConfig.telegramConfig.chatId + "\" -F \"photo=@saved_imgs\\" + filename + "\" \\ https://api.telegram.org/bot" + programConfig.telegramConfig.apiKey + "/sendphoto";
                system(ce.c_str());
            }
        }

        std::this_thread::sleep_for(chrono::milliseconds(3000));
        //Sleep(3000);
    }
}

void inline ChangeNotificationState(HWND hwndl, HMODULE g_hinst, NISTATE state, const char* msg, const char* title) {
    int maxTries = 4;
    int count = 0;
    while (!SetStateNotificationIcon(hwndl, g_hinst, state, msg, title) && count <= maxTries) {
        std::cout << "Trying again" << std::endl;
        std::this_thread::sleep_for(chrono::milliseconds(1));        
        count++;
    }
    if (count >= maxTries)
        std::cout << "Failed to send notif." << std::endl;
}

/// <summary> Takes a frame from each camera then stack and display them in a window </summary>
void ShowFrames(CameraConfig* configs, const int amountCameras, 
                ushort interval, bool& stop, HWND hwndl, HMODULE g_hinst) {
    // ============
    //  Variables 
    // ============
      
    // saves the frames to show in a iteration
    std::vector<cv::Mat> frames;
    
    // saves the cameras that are ready to be displayed
    std::vector<bool> ready;

    // Windows notification alert message and title
    //const char* notificationMsg = "";
    const char* notificationTitle = "Camera alert!";
    
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
                    ChangeNotificationState(hwndl, g_hinst, currentState, configs[i].cameraName.c_str(), notificationTitle);

                    std::cout << "[W] " << configs[i].cameraName << " detected a person..." << std::endl;
                }
            } else if (configs[i].state == NI_STATE_DETECTING) {
                allCamerasInSentry = false;
                
                // Send notification if the current state != to what the camera state is
                if (currentState != NI_STATE_DETECTING && currentState != NI_STATE_DETECTED) {
                    
                    // change the current state
                    currentState = NI_STATE_DETECTING;

                    // send the notification
                    ChangeNotificationState(hwndl, g_hinst, currentState, configs[i].cameraName.c_str(), notificationTitle);

                    std::cout << "[I] " << configs[i].cameraName << " is trying to match a person in the frame..." << std::endl;
                }
            }else if (configs[i].state == NI_STATE_SENTRY) {
                allCamerasInSentry = allCamerasInSentry && true;
            }     
        }

        if (allCamerasInSentry && currentState != NI_STATE_SENTRY) {
            currentState = NI_STATE_SENTRY;

            ChangeNotificationState(hwndl, g_hinst, currentState, "Sentry camera has something.", notificationTitle);

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

            cv::imshow("TEST", res);
            if (cv::waitKey(1) >= 0) {
                stop = true;
                break;
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

        std::this_thread::sleep_for(chrono::milliseconds(int(interval * 0.7)));
    }
}

///<param name='interval'>Minimum distance (in ms) between frame</param>
void ReadFramesWithIntervals(CameraConfig* config, bool& stop, ushort interval, 
                             bool& somethingDetected, float secondsBetweenImage,
                             bool showPreview, cv::HOGDescriptor& hog) {
    #pragma region SetupVideoCapture

    ushort framesLeft = 0; // amount of frames left to search a person.
    
    // higher interval -> lower max & lower interval -> higher max
    const ushort maxFramesLeft = (100 / interval) * 70; // 100 ms => max = 70 frames

    const int x = config->roi.point1.x;
    const int h = abs(config->roi.point2.y - config->roi.point1.y);
    const int w = abs(config->roi.point2.x - config->roi.point1.x);


    const char* camName = &config->cameraName[0];
    const CAMERATYPE camType = config->type;

    int totalNonZeroPixels = 0;
    const uint8_t framesToRecognice = (100 / interval) * 30; // amount of frame that recognition will be active before going to idle state
    const int frameArea = w * h;
    //const int maxNonZeroPixels = frameArea * ((config->sensibility + 0.0) / 100); // Max non zero to leave idle state

    cv::Mat lastFrame;
    cv::Mat diff;

    //hog.winSize = cv::Size(w, h);

    cv::VideoCapture capture(config->url);

    std::cout << "Opening " << camName << "..." << endl;
    assert(capture.isOpened());

    cv::Mat frame;
    cv::Mat invalid;
    cv::Mat frameToShow;

#pragma endregion

    auto timeLastframe = high_resolution_clock::now();
    bool newFrame = false;

    /* Image saver */
    auto timeLastSavedImage = high_resolution_clock::now();

    while (!stop && capture.isOpened()) {
        auto now = high_resolution_clock::now();

        const NISTATE camState = config->state;

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
                std::cout << "Received a invalid frame from \"" << camName << "\". Restarting connection..." << std::endl;

                capture.release();
                capture.open(config->url);

                //framesCount--;
                assert(capture.isOpened());
                std::cout << "Connected to \"" << camName << "\" successfully...." << std::endl;
                continue;
            }

            cv::resize(frame, frame, RESIZERESOLUTION);

            if (showPreview)
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
                totalNonZeroPixels = cv::countNonZero(diff);
            }

            lastFrame = frame;

            // take a percentage of the frame area
            double percentage = ((config->sensibility + 0.0) / 100);
            if (totalNonZeroPixels > frameArea * percentage) {
                if (camState != NI_STATE_DETECTED
                    && camType != CAMERA_SENTRY
                    && camState != NI_STATE_DETECTING)
                    config->state = NI_STATE_DETECTING; // Change the state of the camera

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
                if(showPreview)
                    config->frames.push_back(frameToShow);
            } else {
                std::vector<cv::Rect> detections;
                vector< double > foundWeights;
                if (RECOGNICEALWAYS || framesLeft > 0) {
                    hog.detectMultiScale(frame, detections, foundWeights, config->hitThreshold, cv::Size(8, 8), cv::Size(4, 4), 1.05);
                    for (size_t i = 0; i < detections.size(); i++) {
                        detections[i].x += x;
                        cv::Scalar color = cv::Scalar(0, foundWeights[i] * foundWeights[i] * 200, 0);
                        cv::rectangle(frameToShow, detections[i], color);
                    }

                    auto now = high_resolution_clock::now();;
                    auto time = (now - timeLastSavedImage) / std::chrono::milliseconds(1);
                    if (detections.size() > 0 && time >= secondsBetweenImage * 1000) {
                        config->state = NI_STATE_DETECTED;

                        somethingDetected = true;

                        std::string date = Utils::GetTimeFormated();

                        config->framesToUpload.push_back(std::tuple<cv::Mat, std::string>(frameToShow, date));

                        timeLastSavedImage = high_resolution_clock::now();
                    }

                    framesLeft--;
                    std::cout << camName << " -- Frames " << framesLeft << std::endl;
                }
#pragma endregion

                if (framesLeft == 0) {
                    config->state = NI_STATE_SENTRY;
                }

                newFrame = false;

                if (showPreview)
                    config->frames.push_back(std::move(frameToShow));
            }
        }
    }

    std::cout << "Closed connection with " << camName << std::endl;

    capture.release();
}

void CheckTimeSensibility(CameraConfig* config, const int configSize, bool& stop) {
    int secondSleep = 15;
    int minutesUntilNextHour = 0;
    int secondsElapsed = 0;
    while (!stop) {
        if (secondsElapsed / 60 >= minutesUntilNextHour) {
            int hour = Utils::GetCurrentHour(std::ref(minutesUntilNextHour));
            bool changeSens = false;
            // run over each camera configuration
            for (size_t i = 0; i < configSize; i++) {
                int sensSize = config[i].sensibilityList.size();
                // run over each sensibility config
                for (size_t j = 0; j < sensSize; j++) {
                    Time sens = config[i].sensibilityList[j];
                    if (sens.sensibility != config[i].sensibility) {
                        // separe the if in two scenarios 
                        // 1. 0----TO----12---FROM---23 => FROM > TO => Only will pass if hour > FROM or hour < to
                        // 2. 0---FROM---12----TO----23 => FROM < TO => Only will pas if is between FROM and TO.
                        if (sens.from > sens.to) {
                            if (hour >= sens.from || hour <= sens.to)
                                changeSens = true;
                        } else {
                            if (hour >= sens.from && hour <= sens.to)
                                changeSens = true;
                        }

                        if (changeSens) {
                            std::cout << "Sensibility changed in " << config[i].cameraName
                                << " from " << config[i].sensibility << " to " << sens.sensibility << std::endl;

                            config[i].sensibility = sens.sensibility;

                            changeSens = false;
                        }
                    }
                }
            }
        }

        //Sleep((minutesUntilNextHour + 2) * 60000);
        std::this_thread::sleep_for(chrono::seconds(secondSleep));
        //Sleep(secondSleep * 1000); // sleep only seconds to be able to check if stop is true
        secondsElapsed += secondSleep;
    }
}

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(int argc, char* argv[]){
    bool isUrl = false;
    char url[200];

    // read for url... this is used for the configurator program.
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-u") == 0) {
                isUrl = true;
            } else if (strlen(argv[i]) > 1 && isUrl) {                
                strcpy_s(url, argv[i]);
            }
        }

        if (isUrl) {
            hash<string> hasher;
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
        
    bool stop = false;
    bool somethingDetected = false;

    std::vector<thread> threads;

    HWND hwnd = GetConsoleWindow();
    HMODULE g_inst = GetModuleHandleA(NULL);

    AddNotificationIcon(hwnd, g_inst);

    // configs
    std::vector<CameraConfig> configs;
    ProgramConfig programConfig;

    // Get the cameras and program configurations
    ConfigFileHelper fhelper;
    fhelper.ReadFile(programConfig, configs);

    int configsSize = configs.size();

    if (configsSize == 0) {
        std::cout << "Couldn't fin cameras in the config file." << configsSize << std::endl;
        std::getchar();
        return 0;
    }

    std::cout << "Cameras found: " << configsSize << std::endl;

    // start hog decriptor
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    // Start a thread for each camera
    for (size_t i = 0; i < configsSize; i++) {
        threads.push_back(std::thread(ReadFramesWithIntervals, &configs[i], std::ref(stop), programConfig.msBetweenFrame, std::ref(somethingDetected), programConfig.secondsBetweenImage, programConfig.showPreview, std::ref(hog)));
    }

    if (programConfig.showPreview)
        // Start the thread to show the images captured.
        threads.push_back(std::thread(ShowFrames, &configs[0], configsSize, programConfig.msBetweenFrame, std::ref(stop), hwnd, g_inst));

    // Start a thread for save and upload the images captured    
    threads.push_back(std::thread(SaveAndUploadImage, &configs[0], configsSize, std::ref(programConfig),  std::ref(stop)));

    // Start a thread for save and upload the images captured    
    threads.push_back(std::thread(CheckTimeSensibility, &configs[0], configsSize, std::ref(stop)));

    if (!programConfig.showPreview) {
        std::cout << "Press a key to stop the program.\n";
        std::getchar();
        stop = true;
    }

    // Wait until every thread finish
    int szThreads = threads.size();
    for (int i = 0; i < szThreads; i++) {
        if (i == szThreads - 1)
            std::cout << "Please wait... 15 seconds until release all the resources used." << std::endl;
        threads[i].join();     
    }

    DeleteNotificationIcon();
}