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

#define RESIZERESOLUTION cv::Size(RES_WIDTH, RES_HEIGHT)
#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std;
using namespace chrono;

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

void SaveAndUploadImage(CameraConfig* configs, const int amountCameras, bool& stop) {
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

                const std::string ce = "curl -F \"chat_id=" + BOT_CHAT_ID + "\" -F \"photo=@saved_imgs\\" + filename + "\" \\ https://api.telegram.org/bot" + BOT_TOKEN + "/sendphoto";
                system(ce.c_str());
            }
        }

        Sleep(3000);
    }
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
                if (currentState != NI_STATE_DETECTED) {
                    currentState = NI_STATE_DETECTED;
                    if (!SetStateNotificationIcon(hwndl, g_hinst, currentState))
                        SetStateNotificationIcon(hwndl, g_hinst, currentState); // try again
                    std::cout << "[W] " << configs[i].cameraName << " detected a person..." << std::endl;
                }
            } else if (configs[i].state == NI_STATE_DETECTING) {
                allCamerasInSentry = false;
                if (currentState != NI_STATE_DETECTING && currentState != NI_STATE_DETECTED) {
                    currentState = NI_STATE_DETECTING;
                    if (!SetStateNotificationIcon(hwndl, g_hinst, currentState))
                        SetStateNotificationIcon(hwndl, g_hinst, currentState); // try again
                    std::cout << "[I] " << configs[i].cameraName << " is trying to match a person in the frame..." << std::endl;
                }
            } else if (configs[i].state == NI_STATE_SENTRY) {
                allCamerasInSentry = allCamerasInSentry && true;
            }
        }

        if (allCamerasInSentry && currentState != NI_STATE_SENTRY) {
            currentState = NI_STATE_SENTRY;

            if (!SetStateNotificationIcon(hwndl, g_hinst, currentState)) {
                std::cout << "Trying again" << std::endl;
                SetStateNotificationIcon(hwndl, g_hinst, currentState); // try again
            }

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

        Sleep(interval * 0.7);
    }
}

///<param name='interval'>Minimum distance (in ms) between frame</param>
void ReadFramesWithIntervals(CameraConfig* config, bool& stop, ushort interval, bool& somethingDetected, float secondsBetweenImage) {
    #pragma region SetupVideoCapture

    ushort framesLeft = 0; // amount of frames left to search a person.
    
    // higher interval -> lower max & lower interval -> higher max
    const ushort maxFramesLeft = (100 / interval) * 70; // 100 ms => max = 70 frames

    const int x = config->roi.point1.x;
    const int h = abs(config->roi.point2.y - config->roi.point1.y);
    const int w = abs(config->roi.point2.x - config->roi.point1.x);

#if !RECOGNICEALWAYS
    int totalNonZeroPixels = 0;
    const uint8_t framesToRecognice = (100 / interval) * 30; // amount of frame that recognition will be active before going to idle state
    const int frameArea = w * h;
    //const int maxNonZeroPixels = frameArea * ((config->sensibility + 0.0) / 100); // Max non zero to leave idle state

    cv::Mat lastFrame;
    cv::Mat diff;
#endif

    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    //hog.winSize = cv::Size(w, h);

    cv::VideoCapture capture(config->url);

    std::cout << "Opening " << config->cameraName << "..." << endl;
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
                std::cout << "Received a invalid frame from \"" << config->cameraName << "\". Restarting connection..." << std::endl;

                capture.release();
                capture.open(config->url);

                //framesCount--;
                assert(capture.isOpened());
                std::cout << "Connected to \"" << config->cameraName << "\" successfully...." << std::endl;
                continue;
            }

            cv::resize(frame, frame, RESIZERESOLUTION);

            frameToShow = frame.clone();

            // Take the region of interes
            if (!config->roi.isEmpty()) {
                cv::Rect roi(config->roi.point1, config->roi.point2);
                frame = frame(roi);
            }

            // Then rotate it
            if (config->rotation != 0) ImageManipulation::RotateImage(frame, config->rotation);

            cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

#if !RECOGNICEALWAYS
            if (lastFrame.rows == RESIZERESOLUTION.height) {
                cv::absdiff(lastFrame, frame, diff);
                totalNonZeroPixels = cv::countNonZero(diff);
            }

            lastFrame = frame;

            // take a percentage of the frame area
            if (totalNonZeroPixels > frameArea * ((config->sensibility + 0.0) / 100)) {
                if (config->state != NI_STATE_DETECTED && config->state != NI_STATE_DETECTING)
                    config->state = NI_STATE_DETECTING; // Change the state of the camera

                // Increment frames left
                if (framesLeft < maxFramesLeft)
                    framesLeft += framesToRecognice;
            }
#endif

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
                
#if !RECOGNICEALWAYS
                framesLeft--;
#endif
                std::cout << config->cameraName << " -- Frames " << framesLeft << std::endl;
            }
            #pragma endregion

            if (framesLeft == 0) {
                config->state = NI_STATE_SENTRY;
            }

            newFrame = false;
#if SHOWFRAMEINSCREEN
            config->frames.push_back(frameToShow);
#endif
        }
    }

    std::cout << "Closed connection with " << config->cameraName << std::endl;

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
        Sleep(secondSleep * 1000); // sleep only seconds to be able to check if stop is true
        secondsElapsed += secondSleep;
    }
}

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(int argc, char* argv[]){
    bool isUrl = false;
    std::string url = "";

    // read for url... this is used for the configurator program.
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-u") == 0) {
                isUrl = true;
            } else if (strlen(argv[i]) > 1 && isUrl) {
                url = argv[i];
            }
        }

        if (isUrl) {
            hash<string> hasher;
            size_t name = hasher(url);
            string path = std::to_string(name) + ".jpg";

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

    std::cout << "Cameras found: " << configsSize << std::endl;

    // Start a thread for each camera
    for (size_t i = 0; i < configsSize; i++) {
        threads.push_back(std::thread(ReadFramesWithIntervals, &configs[i], std::ref(stop), programConfig.msBetweenFrame, std::ref(somethingDetected), programConfig.secondsBetweenImage));
    }

    // Start the thread to show the images captured.
    threads.push_back(std::thread(ShowFrames, &configs[0], configsSize, programConfig.msBetweenFrame, std::ref(stop), hwnd, g_inst));

    // Start a thread for save and upload the images captured    
    threads.push_back(std::thread(SaveAndUploadImage, &configs[0], configsSize, std::ref(stop)));

    // Start a thread for save and upload the images captured    
    threads.push_back(std::thread(CheckTimeSensibility, &configs[0], configsSize, std::ref(stop)));

    // Wait until every thread finish
    for (size_t i = 0; i < threads.size(); i++) {
        threads[i].join();
        if (i == 0)
            std::cout << "Please wait... 15 seconds until release all the resources used." << std::endl;        
    }

    DeleteNotificationIcon();
}