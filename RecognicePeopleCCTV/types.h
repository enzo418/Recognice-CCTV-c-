#pragma once

#define RES_WIDTH 640
#define RES_HEIGHT 360

typedef unsigned short int ushort;
typedef unsigned long int ulong;

struct CameraConfig{
	std::string cameraName;
	std::string url;
	cv::Point roi[2];
	double hitThreshold;
	int order;
	int sensibility;
	int rotation;

	std::vector<cv::Mat> frames; // frame that the Video Capturer read
	std::vector<std::tuple<cv::Mat, std::string>> framesToUpload; // the frame wich a person was detected. 
};

