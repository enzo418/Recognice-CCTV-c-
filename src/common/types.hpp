#pragma once
#ifdef WINDOWS
#include "../windows/NotificationIconWindows.hpp"
#else
#include "../unix/NotificationIconUnix.hpp"
#define strcpy_s(x,y) strcpy(x,y)
#define HWND long*
#define HMODULE long*
#endif
#include <opencv2/core/base.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

typedef unsigned short int ushort;
typedef unsigned long int ulong;

// ===========================
//  ActionMessage definitions
// ===========================

#define MESSAGE_SIZE 150

class Message {
	public:
	cv::Mat image;
	char text[MESSAGE_SIZE];

	// A message can be just text
	Message(const char* message) {
		snprintf(text, MESSAGE_SIZE, "%s", message);
		isText = true;
	}

	// Or a image with a date
	Message(cv::Mat img, const char* date){
		image = img;
		std::cout << "date => " << date << std::endl;
		std::cout << " 3 Frame size " << image.rows << ", " << image.cols << std::endl;
		//sprintf_s(text, "%s", date);
		snprintf(text, MESSAGE_SIZE, "%s", date);
	}

	// Or a action (play sound)
	Message() { 
		isSound = true; 
		strcpy_s(text, ""); 
	};

	bool IsText() {
		return isText;
	}

	bool IsSound() {
		return isSound;
	}

	private:
	bool isSound = false;
	bool isText = false;
};

typedef std::vector<Message> MessageArray;

// ====================
//  Camera definitions
// ====================

#define RES_WIDTH 640
#define RES_HEIGHT 360

// START ROI def

class ROI {
public:
	cv::Point point1;
	cv::Point point2;

	ROI() {
		point1 = cv::Point(0, 0);
		point2 = cv::Point(RES_WIDTH, RES_HEIGHT);
	};

	bool isEmpty() {
		return (point1.x == 0 && point1.y == 0 && point2.x == 0 && point2.y == 0);
	};
};

// END ROI def

typedef unsigned char CAMERATYPE;

/*	CameraType
	Sentry state means that the camera is only will compare the current frame to the last one
	to see if there is a significant change. And if it's it only will send the notifications.
*/
#define CAMERA_SENTRY 1

/*	CameraType
	Active type is sentry + when it detects any significant change it tries to math a person
	in the frame. If success we will send all the notifications.
*/
#define CAMERA_ACTIVE 2

/*	CameraType
	Disabled type is activated to disable a camera.
*/
#define CAMERA_DISABLED 0

struct CameraConfig {
	std::string cameraName;

	// url or path to the camera / video
	std::string url;

	// Region of interest
	ROI roi;

	// Threshold to recognice that a match/detection is valid
	double hitThreshold;

	// Used to show the cameras sorted by order
	int order;

	// rotation of the camera
	int rotation;

	// pixels that must change to change state to detecting
	int changeThreshold;

	// Current state of the camera sentry, detecting or detected.
	NISTATE state;

	// The type of the camera. See definition.
	CAMERATYPE type;

	// List of frames captured
	std::vector<cv::Mat> frames;

	// threshold to use when removing the noise of the frame
	double noiseThreshold;
};

// to be able to sort the array of configs
struct {
	inline bool operator() (const CameraConfig& struct1, const CameraConfig& struct2) {
		return (struct1.order < struct2.order);
	}
}less_than_order;

// ===============
//  Program types
// ===============

struct TelegramBotConfig {
	// telefram bot API
	std::string apiKey;

	// channel to send the images
	std::string chatId;
};

struct ProgramConfig {
	// milliseconds to wait until get a new frame from the camera
	ushort msBetweenFrame;

	// seconds to wait until save a new frame to the fraesToUpload list of the camera.
	float secondsBetweenImage;

	// seconds to waait until send another message.
	int secondsBetweenMessage;

	// if should send a image when the threshold change was passed.
	bool sendImageWhenDetectChange;

	// resolution of the preview
	cv::Size outputResolution;

	// telefram bot config
	TelegramBotConfig telegramConfig;

	// show preview of the cameras
	bool showPreview;

	// if should show in the preview the area that the camera sees
	bool showAreaCameraSees;

	// if should show the processed images with applied roation, threshold, ...
	bool showProcessedFrames;
};
