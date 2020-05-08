#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include "ImageManipulation.h"
#include <Windows.h>
#include <vector>
#include <thread>
#include <map>
#include <chrono>

#define RESIZERESOLUTION cv::Size(640, 360)
#define RECOGNICEALWAYS false

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

void ShowFrames(CameraConfig* configs, int amountCameras, bool& stop) {
    std::vector<cv::Mat> frames;
    bool ready[2];
    uint8_t size = 0;
    uint8_t stackHSize = 2;
    cv::Mat res;

    frames.resize(amountCameras);
    
    for (size_t i = 0; i < amountCameras; i++)
        ready[i] = false;    
    
    while (!stop) {
        for (size_t i = 0; i < amountCameras; i++) {
            if (configs[i].frames.size() > 0) {
                // if the vector in i has no frame
                if (!ready[configs[i].order]) {
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
            //frames.erase(frames.begin(), frames.end());
            size = 0;

            for (size_t i = 0; i < amountCameras; i++) {
                ready[i] = false;
            }

            cv::imshow("TEST", res);
            if (cv::waitKey(1) >= 0) {
                stop = true;
            }
        }
        
        Sleep(0.5);
    }
}

void Process(CameraConfig* config, bool& stop) {
    int framesLeft = 50; // amount of frames left to search a person.

    const int x = config->roi[0].x;
    const int h = abs(config->roi[1].y - config->roi[0].y);
    const int w = abs(config->roi[1].x - config->roi[0].x);
#if !RECOGNICEALWAYS
    int totalNonZeroPixels = 0;
    const uint8_t framesToRecognice = 150; // amount of frame that recognition will be active before going to idle state
    const int maxNonZeroPixels = round((w * h) * ((config->sensibility + 0.0)/ 100)); // Max non zero to leave idle state

    cv::Mat lastFrame;
    cv::Mat diff;
#endif

    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    //hog.winSize = cv::Size(w, h);

    cv::VideoCapture capture(config->url);

    if (!capture.isOpened()) {
        throw "Error: Couldn't open  camera " + config->cameraName + " rtsp.";
    }

    cv::Mat frame;
    cv::Mat frameToShow;

    while (!stop && capture.isOpened()) {
        if (!capture.read(frame)) {
            //Error
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
        if (lastFrame.data) {
            cv::absdiff(lastFrame, frame, diff);
            totalNonZeroPixels = cv::countNonZero(diff);
        }

        lastFrame = frame;

        //cout << maxNonZeroPixels - totalNonZeroPixels << endl;
        if (totalNonZeroPixels > maxNonZeroPixels) {
            if (framesLeft < 500)
                framesLeft += framesToRecognice;
        } else if (framesLeft == 0) {

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
            /*if (detections.size() > 0)
                cout << "Detections " << detections.size() << endl;*/
            cout << config->cameraName << " -- Frames "<< framesLeft << endl;
#if !RECOGNICEALWAYS
            framesLeft--;
#endif
        }

        config->frames.push_back(frameToShow);
    }

    capture.release();
}

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(){
    bool stop = false;
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
        
    //configs.shrink_to_fit(); // Requests the removal of unused capacity

    std::thread t1(Process, &configs[0], std::ref(stop));
    std::thread t2(Process, &configs[1], std::ref(stop));

    ShowFrames(&configs[0], 2, stop);

    //Sleep(500);

    //CameraConfig d = configs[1];

    stop = true;
    t1.join();
    t2.join();


    //Process(config1, stop);
    //RecordSampleOfCamera(config1);

    //std::getchar();
}