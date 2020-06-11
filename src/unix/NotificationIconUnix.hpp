#pragma once

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

bool AddNotificationIcon();
bool SetStateNotificationIcon();
bool PlayNotificationSound();
bool DeleteNotificationIcon();