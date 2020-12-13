#pragma once

#include "../ChangeDescriptor/ChangeDescriptor.hpp"

#include <opencv2/core/base.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <chrono>

#include <iostream>
#include <vector>
#include <string>

// Notification Icon State
typedef unsigned char NISTATE;

/*	NISTATE:
	Sentry state means that the camera is only comparing the current frame to the last one
	to see if there is a significant change.
*/
#define NI_STATE_SENTRY 1 

/*	NISTATE:
	Detecting state means that the camera is currently using a classification method to
	detect a person on the frame.
*/
#define NI_STATE_DETECTING 2

/*	NISTATE:
	Detected means that the camera sucesfully detected person on the frame.
*/
#define NI_STATE_DETECTED 3

typedef unsigned short int ushort;
typedef unsigned long int ulong;

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

enum {
	CAMERA_DISABLED = 0, 	// Disabled type is activated to disable a camera.
	
	CAMERA_SENTRY, 			// Sentry state means that the camera is only will compare the current frame to the last one
							// to see if there is a significant change. And if it's it only will send the notifications.

	CAMERA_ACTIVE 			// Active type is sentry + when it detects any significant change it tries to math a person
							// in the frame. If success we will send all the notifications.
};


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

// struct UntrustedFinding {
// 	FindingInfo finding;
	
// 	// Value to indecate trust, min is -128 and max 127
// 	char trust;
// };