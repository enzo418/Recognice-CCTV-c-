#pragma once

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

	std::vector<cv::Mat> frames;
};

