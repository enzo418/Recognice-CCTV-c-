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

#include <chrono>

#include <iostream>
#include <vector>
#include <string>

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
	char caption[MESSAGE_SIZE];

	// A message can be just text
	Message(const char* message) {
		snprintf(text, MESSAGE_SIZE, "%s", message);
		isText = true;
	}

	// Or a image with a date
	Message(cv::Mat img, const char* filename, const char* imageCaption){
		image = img;
		std::cout << " filename => " << filename << std::endl;
		std::cout << " 3 Frame size " << image.rows << ", " << image.cols << std::endl;
		//sprintf_s(text, "%s", date);
		snprintf(text, MESSAGE_SIZE, "%s", filename);

		snprintf(caption, MESSAGE_SIZE, "%s", imageCaption);
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

// =====================
//  Register Definition
// =====================

enum RegisterPoint {entryPoint, exitPoint};

// registers the person that enters or leaves the site
struct Register{
	// numerical identifier
	size_t id;

	// saves if the person was entering or leaving the site
	RegisterPoint firstPoint;
	
	// saves the time (Unix Time) used to know if the timeout was reached
	tm* time_point;
	
	// flag to know if this register was finished (could find the next point or we reached the timeout)
	bool finished;	

	// saves the id of the partner (the other register), size_t max if it was not valid.
	size_t partner;

	// afters is finished saves the last point where the person was seen.
	RegisterPoint lastPoint;
};


// ====================
//  Camera definitions
// ====================

#define RES_WIDTH 640
#define RES_HEIGHT 360

#define RESIZERESOLUTION cv::Size(RES_WIDTH, RES_HEIGHT)

// Region of interest
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


struct AreasDelimiters {
	// area to consider that the person is in the "entry point"
	cv::Rect rectEntry;
	// area to consider that the person is in the "exit point"
	cv::Rect rectExit;

	AreasDelimiters(){
		
	};

	AreasDelimiters(cv::Rect _entry, cv::Rect _exit) : rectEntry(_entry), rectExit(_exit) {
		
	};
};

// ===============
//  Program types
// ===============

struct TelegramBotConfig {
	bool useTelegramBot;
	
	// telefram bot API
	std::string apiKey;

	// channel to send the images
	std::string chatId;
};