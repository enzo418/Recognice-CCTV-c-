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

void SaveAndUploadImage(CameraConfig* configs, const int amountCameras, bool& somethingDetected, bool& stop) {
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

void ShowFrames(CameraConfig* configs, const int amountCameras, ushort interval, bool& somethingDetected, bool& stop) {
    /* variables */
    std::vector<cv::Mat> frames;
    std::vector<bool> ready;
    uint8_t size = 0;
    uint8_t stackHSize = amountCameras > 1 ? 2 : 1;
    cv::Mat res;

    bool isFirstIteration = true;

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
void ReadFramesWithIntervals(CameraConfig* config, bool& stop, ushort interval, bool& somethingDetected) {
    #pragma region SetupVideoCapture

    ushort framesLeft = 0; // amount of frames left to search a person.
    
    // higher interval -> lower max & lower interval -> higher max
    const ushort maxFramesLeft = (100 / interval) * 70; // 100 ms => max = 70 frames

    const int x = config->roi[0].x;
    const int h = abs(config->roi[1].y - config->roi[0].y);
    const int w = abs(config->roi[1].x - config->roi[0].x);

#if !RECOGNICEALWAYS
    int totalNonZeroPixels = 0;
    const uint8_t framesToRecognice = (100 / interval) * 30; // amount of frame that recognition will be active before going to idle state
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
        
    auto timeLastframe = high_resolution_clock::now();
    bool newFrame = false;  

    /* Image saver */
    auto timeLastSavedImage = high_resolution_clock::now();
    ushort secondsBetweenImage = 2;

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
                cout << "Received a invalid frame from \"" << config->cameraName << "\". Restarting connection..." << endl;
                
                capture.release();
                capture.open(config->url);
                
                //framesCount--;
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

                auto now = high_resolution_clock::now();;
                auto time = (now - timeLastSavedImage) / std::chrono::milliseconds(1);
                if (detections.size() > 0 && time >= (int)secondsBetweenImage * 1000) {                      
                    somethingDetected = true;

                    std::string date = GetTimeFormated();

                    config->framesToUpload.push_back(std::tuple<cv::Mat, std::string>(frameToShow, date));

                    timeLastSavedImage = high_resolution_clock::now();
                }

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
        
    // each time you reduce 50 ms cpu usage increases by 3 to 6 percent.
    ushort interval = 80; // ms
    
    std::thread t1(ReadFramesWithIntervals, &configs[0], std::ref(stop), interval, std::ref(somethingDetected));
    std::thread t2(ReadFramesWithIntervals, &configs[1], std::ref(stop), interval, std::ref(somethingDetected));

#if SHOWFRAMEINSCREEN
    std::thread t3(ShowFrames, &configs[0], 2, interval, std::ref(somethingDetected), std::ref(stop));
#else
    std::getchar();
#endif

    std::thread t4(SaveAndUploadImage, &configs[0], 2, std::ref(somethingDetected), std::ref(stop));

    t1.join();
    t2.join();
    t3.join();
    t4.join();
}