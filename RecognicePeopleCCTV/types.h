#pragma once
#include "NotificationIcon.h"
#include <opencv2\core\base.hpp>
#include <opencv2\core\types.hpp>

#define RES_WIDTH 640
#define RES_HEIGHT 360

typedef unsigned short int ushort;
typedef unsigned long int ulong;

typedef bool* FLAG;

class ROI {
public:
	cv::Point point1;
	cv::Point point2;

	ROI() {};

	bool isEmpty() {
		return (point1.x == 0 && point1.y == 0 && point2.x == 0 && point2.y == 0);
	};
};

class Time {
public:
	int from, to, sensibility;

	Time(int from, int to, int sensibility) : from(from), to(to), sensibility(sensibility) {};
	Time() {
		from = 0;
		to = 0;
		sensibility = 0;
	};
};

struct CameraConfig{
	std::string cameraName;
	
	// url or path to the camera / video
	std::string url;
	
	// Region of interest
	ROI roi;

	// Threshold to recognice that a match/detection is valid
	double hitThreshold;

	// Used to show the cameras sorted by order
	int order;

	// (0-100) percentage of pixels needed to start the classification in the frame
	int sensibility;

	// rotation of the camera
	int rotation;

	// Current state of the camera sentry, detecting or detected.
	NISTATE state;

	// List of "sensibilities" to set depending the current time
	std::vector<Time> sensibilityList;

	// List of frames captured
	std::vector<cv::Mat> frames;

	// List of frames, where something was detected, to save in the computer and send to the bot.
	std::vector<std::tuple<cv::Mat, std::string>> framesToUpload;
};

struct ProgramConfig {
	// milliseconds to wait until get a new frame from the camera
	ushort msBetweenFrame;

	// seconds to wait until save a new frame to the fraesToUpload list of the camera.
	float secondsBetweenImage;
};