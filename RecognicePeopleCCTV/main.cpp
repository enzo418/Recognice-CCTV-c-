#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include "ImageManipulation.h"
#include <Windows.h>
#include <vector>

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std;

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(){
    cv::Mat image;
    image = cv::imread("test.png", cv::IMREAD_COLOR); // Read the file

    cv::Mat image1;
    image1 = cv::imread("test_2.png", cv::IMREAD_COLOR); // Read the file

    if (!image.data) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    /*std::vector<cv::Mat> matArray = { image, image1};
    cv::Mat* ims = &matArray[0];
    cv::Mat res = ImageManipulation::VStackWithPad(ims, 2);*/

    // roi image
    cv::Rect roi(cv::Point(204, 24), cv::Point(346, 142));
    image = image(roi);
        
    std::vector<cv::Mat> matArray = { image, image1};
    cv::Mat* ims = &matArray[0];
    cv::Mat res = ImageManipulation::StackImages(ims, 2, 1);

    //cv::imshow("Display", res); // Show our image inside it. 
       
    //cv::waitKey(0); // Wait for a keystroke in the window

    cv::VideoCapture capture("rtsp://192.168.1.18:554/user=admin&password=d12&channel=4&stream=0.sdp");

    if (!capture.isOpened()) {
        return -1;
    }

    cv::Mat frame;

    while (capture.isOpened()) {
        if (!capture.read(frame)) {
            //Error
        }

        cv::resize(frame, frame, cv::Size(640, 360));

        cv::imshow("TEST", frame);

        if (cv::waitKey(1) >= 0) break;
        //cv::waitKey(0);
    }

    capture.release();

    cv::destroyWindow("TEST");

    //std::getchar();
}