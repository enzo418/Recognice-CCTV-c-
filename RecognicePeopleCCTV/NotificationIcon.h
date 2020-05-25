#pragma once
#include <windows.h>

// Use a guid to uniquely identify our icon
class __declspec(uuid("9D0BAB92-4E1C-488e-A1E1-2331AAAE2CB5")) NotificationIcon;

// Start from messages > 1024 (<= 1024 are reserved by windows)
#define WM_MYMESSAGE (WM_USER + 1)

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

BOOL AddNotificationIcon(HWND hwnd, HMODULE g_hInst);
BOOL SetStateNotificationIcon(HWND hWnd, HMODULE g_hInst, NISTATE state, const char* msg, const char* title);
BOOL PlayNotificationSound();
BOOL DeleteNotificationIcon();