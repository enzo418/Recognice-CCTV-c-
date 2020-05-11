#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include "ImageManipulation.h"
#include <Windows.h>
#include <vector>
#include <thread>
#include <map>
#include <chrono>

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

std::string GetTimeFormated() {
    time_t rawtime;
    struct tm timeinfo;
    char buffer[80];

    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    strftime(buffer, 80, "%d_%m_%Y_%H_%M_%S", &timeinfo);
    return buffer;
};

void ShowFrames(CameraConfig* configs, const int amountCameras, ushort interval, bool& somethingDetected, bool& stop) {
    /* variables */
    std::vector<cv::Mat> frames;
    double avgBufferSize = 0;
    std::vector<bool> ready;
    uint8_t size = 0;
    uint8_t stackHSize = amountCameras > 1 ? 2 : 1;
    cv::Mat res;

    frames.resize(amountCameras);

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

    /* DEBUG */
    auto timeLastPrint = high_resolution_clock::now();
    ushort debugged = amountCameras;

    while (!stop) {
        avgBufferSize = 0;
        for (size_t i = 0; i < amountCameras; i++) {
            if (configs[i].frames.size() > 0) {
                // if the vector in i has no frame
                if (!ready[configs[i].order]) {
                    avgBufferSize += configs[i].frames.size();

                    /*DEBUG*/
                    auto now = high_resolution_clock::now();
                    auto time = (now - timeLastPrint) / std::chrono::milliseconds(1);

                    if (time > 60 * 1000) {
                        cout << configs[i].cameraName << " buffer size: " << configs[i].frames.size() << endl;
                        debugged--;
                        if (debugged == 0) {
                            timeLastPrint = high_resolution_clock::now();
                            debugged = amountCameras;
                        }
                    }
                    /*END DEBUG*/

                    // take the first frame and delete it
                    frames[configs[i].order] = configs[i].frames[0];

                    configs[i].frames.erase(configs[i].frames.begin());
                    
                    ready[configs[i].order] = true;
                    
                    size++;
                }
            }
        }
        
        if (size == amountCameras) {
            res = ImageManipulation::StackImages(&frames[0], size, stackHSize);
            size = 0;
                        
            /* Uncomment if want to record video
            if (frameSize.width != res.cols) {
                frameSize.width = res.cols;
                frameSize.height = res.rows;
            }
            */

            for (size_t i = 0; i < amountCameras; i++) {
                ready[i] = false;
            }

            if (somethingDetected) {
                auto now = high_resolution_clock::now();;
                auto time = (now - timeLastSavedImage) / std::chrono::milliseconds(1);
                if (time >= (int)secondsBetweenImage * 1000) {
                    std::string date = GetTimeFormated();
                    cv::imwrite("saved_imgs/img_" + date + ".jpg", res);
                    timeLastSavedImage = high_resolution_clock::now();
                    somethingDetected = false;
                }
            }

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

        avgBufferSize /= amountCameras;

        Sleep(interval * 0.7);
    }
}

// reduces cpu usage from between 1 and 3 %
///<param name='fps'>How many frames should take per second</param>
///<param name='interval'>Minimum distance (in ms) between frame</param>
void ReadFramesWithIntervals(CameraConfig* config, bool& stop, ushort fps, ushort interval, bool& somethingDetected) {
    #pragma region SetupVideoCapture

    ushort framesLeft = 0; // amount of frames left to search a person.
    const ushort maxFramesLeft = fps * 50;

    const int x = config->roi[0].x;
    const int h = abs(config->roi[1].y - config->roi[0].y);
    const int w = abs(config->roi[1].x - config->roi[0].x);

#if !RECOGNICEALWAYS
    int totalNonZeroPixels = 0;
    const uint8_t framesToRecognice = fps * 10; // amount of frame that recognition will be active before going to idle state
    const int maxNonZeroPixels = round((w * h) * ((config->sensibility + 0.0) / 100)); // Max non zero to leave idle state

    cv::Mat lastFrame;
    cv::Mat diff;
#endif

    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    //hog.winSize = cv::Size(w, h);

    cv::VideoCapture capture(config->url);

    cout << "Opening " << config->cameraName << "..." << endl;
    assert(capture.isOpened());

    cv::Mat frame;
    cv::Mat invalid;
    cv::Mat frameToShow;

    #pragma endregion

    //auto timeLast2frames = high_resolution_clock::now();
    auto timeLastframe = high_resolution_clock::now();
    int framesCount = 0;
    bool newFrame = false;  
    while (!stop && capture.isOpened()) {                              
        auto now = high_resolution_clock::now();

        //auto time = (now - timeLast2frames) / std::chrono::milliseconds(1);
        auto intervalFrames = (now - timeLastframe) / std::chrono::milliseconds(1);
        if (framesCount < fps && intervalFrames >= interval) {
            capture.read(frame);
            timeLastframe = high_resolution_clock::now();
            framesCount++;
            newFrame = true;
        } else {
            capture.read(invalid); // keep reading to avoid error on VC.
        }

        if (framesCount > 0 && framesCount <= fps && newFrame) {
            if (framesCount == fps) {
                //timeLast2frames = high_resolution_clock::now();
                framesCount = 0;
            }

            #pragma region AnalizeFrame

            //assert(frame.rows != 0); // check if the frame is valid
            if (frame.rows == 0) {
                cout << "Received a invalid frame from \"" << config->cameraName << "\". Restarting connection..." << endl;
                
                capture.release();
                capture.open(config->url);
                
                framesCount--;
                assert(capture.isOpened());
                cout << "Connected to \"" << config->cameraName << "\" successfully...." << endl;
                continue;
            }

            cv::resize(frame, frame, RESIZERESOLUTION);

            frameToShow = frame.clone();

            if (config->rotation > 0) ImageManipulation::RotateImage(frame, config->rotation);

            // Take the region of interes
            cv::Point r = config->roi[0] + config->roi[1];
            if ((r).x != 0 || r.y != 0) {
                cv::Rect roi(config->roi[0], config->roi[1]);
                frame = frame(roi);
            }

            cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

#if !RECOGNICEALWAYS
            if (lastFrame.rows == RESIZERESOLUTION.height) {
                cv::absdiff(lastFrame, frame, diff);
                totalNonZeroPixels = cv::countNonZero(diff);
            }

            lastFrame = frame;

            //cout << maxNonZeroPixels - totalNonZeroPixels << endl;
            if (totalNonZeroPixels > maxNonZeroPixels) {
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
                if (detections.size() > 0)
                    somethingDetected = true;
                cout << config->cameraName << " -- Frames " << framesLeft << endl;
#if !RECOGNICEALWAYS
                framesLeft--;
#endif
            }
            #pragma endregion

            newFrame = false;
#if SHOWFRAMEINSCREEN
            config->frames.push_back(frameToShow);
#endif
        }
    }

    cout << "Closed connection with " << config->cameraName << endl;

    capture.release();
}

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(){
    bool stop = false;
    bool somethingDetected = false;
    CameraConfig configs[2];

    configs[0].cameraName = "camera1";
    configs[0].order = 0;
    configs[0].url = "rtsp://192.168.1.18:554/user=admin&password=d12&channel=4&stream=0.sdp";
    //configs[0].url = configs[0].cameraName + ".avi";
    configs[0].roi[0] = cv::Point(200, 0);
    configs[0].roi[1] = cv::Point(200 + 280, 360);
    configs[0].rotation = 5;
    configs[0].hitThreshold = 0.05;
    //configs[0].sensibility = 72; // videoj
    configs[0].sensibility = 69; // rtsp

    configs[1].cameraName = "camera2";
    configs[1].order = 1;
    configs[1].url = "rtsp://192.168.1.18:554/user=admin&password=d12&channel=3&stream=0.sdp";
    //configs[1].url = configs[1].cameraName + ".avi";    
    configs[1].roi[0] = cv::Point(310, 0);
    configs[1].roi[1] = cv::Point(310 + 150, 360);
    configs[1].rotation = -5;
    configs[1].hitThreshold = 0.05;
    //configs[1].sensibility = 90; // video
    configs[1].sensibility = 86;  //rtsp
        
    ushort fps = 3;
    ushort interval = 100;
    //configs.shrink_to_fit(); // Requests the removal of unused capacity

    //std::thread t1(Process, &configs[0], std::ref(stop));
    //std::thread t2(Process, &configs[1], std::ref(stop));*/
    std::thread t1(ReadFramesWithIntervals, &configs[0], std::ref(stop), fps, interval, std::ref(somethingDetected));
    std::thread t2(ReadFramesWithIntervals, &configs[1], std::ref(stop), fps, interval, std::ref(somethingDetected));

#if SHOWFRAMEINSCREEN
    ShowFrames(&configs[0], 2, interval, std::ref(somethingDetected), stop);
#else
    std::getchar();
#endif
    //Sleep(500);

    //CameraConfig d = configs[1];

    stop = true;
    t1.join();
    t2.join();


    //Process(config1, stop);
    //RecordSampleOfCamera(config1);

    //std::getchar();
}