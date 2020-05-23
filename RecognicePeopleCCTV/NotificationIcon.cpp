#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

#include "NotificationIcon.h"
#include "resource.h"
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <strsafe.h>
#include <cassert>

BOOL AddNotificationIcon(HWND hwnd, HMODULE g_hInst) {
    // Declare and initialize the notification icon
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    
    // A handle to the window that receives notifications associated with an icon in the notification area.        
    nid.hWnd = hwnd; 
    
    nid.uCallbackMessage = WM_MYMESSAGE;

    // load the icon from the resources of the proyect
    LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI_ICON_SENTRY), LIM_SMALL, &nid.hIcon);

    // set the flags
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP;

    // the icon will be identified with the GUID
    nid.guidItem = __uuidof(NotificationIcon);

    // Load the tip msg
    LoadString(g_hInst, IDS_STATE_SENTRY, nid.szTip, ARRAYSIZE(nid.szTip));

    // Add the ni
    Shell_NotifyIcon(NIM_ADD, &nid);

    // NOTIFYICON_VERSION_4 is prefered
    nid.uVersion = NOTIFYICON_VERSION_4;
    return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL SetStateNotificationIcon(HWND hWnd, HMODULE g_hInst, NISTATE state, const char* msg, const char* title) {
    NOTIFYICONDATA nid;

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.guidItem = __uuidof(NotificationIcon);
    
    // Set STATE
    WORD IDS = IDS_STATE_SENTRY;
    WORD IDI = IDI_ICON_SENTRY;
    if (state == NI_STATE_DETECTING) { IDS = IDS_STATE_DETECTING; IDI = IDI_ICON_DECTECTING; }
    else if (state == NI_STATE_DETECTED) { IDS = IDS_STATE_DETECED; IDI = IDI_ICON_DETECTED; }

    /* Uncomment to show a windows notification */
    //LoadString(g_hInst, IDS_, nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle));
    //LoadString(g_hInst, IDS_, nid.szInfo, ARRAYSIZE(nid.szInfo));    

    // load the icon
    LoadIconMetric(g_hInst, MAKEINTRESOURCE(IDI), LIM_SMALL, &nid.hIcon);

    // Load the tip msg
    LoadString(g_hInst, IDS, nid.szTip, ARRAYSIZE(nid.szTip));

    nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP; // NIF_INFO => Windows Notif. Alert

    if (strlen(msg) > 0 && strlen(title) > 0) {
        size_t lengthTitle = 0;
        mbstowcs_s(&lengthTitle, nid.szInfoTitle, title, strlen(title));
        size_t lengthMsg = 0;
        mbstowcs_s(&lengthMsg, nid.szInfo, msg, strlen(msg));
        nid.uFlags |= NIF_INFO;
        nid.dwInfoFlags = NIIF_RESPECT_QUIET_TIME;
    }

    return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL DeleteNotificationIcon() {
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    //nid.uFlags = NIF_GUID;
    nid.guidItem = __uuidof(NotificationIcon);
    return Shell_NotifyIcon(NIM_DELETE, &nid);
}