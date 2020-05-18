#pragma once
#include <opencv2\core\base.hpp>
#include <opencv2\core\types.hpp>

#define RES_WIDTH 640
#define RES_HEIGHT 360

typedef unsigned short int ushort;
typedef unsigned long int ulong;


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
	std::string url;
	ROI roi;
	double hitThreshold;
	int order;
	int sensibility;
	int rotation;

	std::vector<Time> sensibilityList;

	std::vector<cv::Mat> frames; // frame that the Video Capturer read
	std::vector<std::tuple<cv::Mat, std::string>> framesToUpload; // the frame wich a person was detected. 
};

struct ProgramConfig {
	ushort msBetweenFrame;
	float secondsBetweenImage;
};