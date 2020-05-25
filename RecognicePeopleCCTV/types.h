#pragma once
#include "NotificationIcon.h"
#include <opencv2\core\base.hpp>
#include <opencv2\core\types.hpp>

typedef unsigned short int ushort;
typedef unsigned long int ulong;

// ===========================
//  ActionMessage definitions
// ===========================

#define MESSAGE_SIZE 100

class Message {
	public:
	cv::Mat image;
	char text[MESSAGE_SIZE];
	bool playSound = false;

	// A message can be just text
	Message(const char* message) {
		snprintf(text, MESSAGE_SIZE, "%s", message);
	}

	// Or a image with a date
	Message(cv::Mat img, const char* date): image(img) {
		snprintf(text, MESSAGE_SIZE, "%s", date);
	}

	// Or a action (play sound)
	Message() { playSound = true; strcpy_s(text, ""); };

	bool IsText() {
		return image.empty() && strlen(text) > 0;
	}

	bool IsSound() {
		return playSound;
	}
};

typedef std::vector<Message> MessageArray;

// ====================
//  Camera definitions
// ====================

#define RES_WIDTH 640
#define RES_HEIGHT 360

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

	// List of frames, where something was detected, to save in the computer and send to the bot.
	//std::vector<std::tuple<cv::Mat, std::string>> framesToUpload;
};

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

	// telefram bot config
	TelegramBotConfig telegramConfig;

	// show preview of the cameras
	bool showPreview;
};
