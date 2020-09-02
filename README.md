# Recognice people on CCTV


# Todo
~~- save: image and video (of the last minute) after ends detecting someone~~

~~- Add GUI to let the user pick cameras and settings (ROI, sesibility, rotation, order, hitThreshold and name), also fps (1000 / interval). option: Should show a preview.~~

~~- Add try icon to show the current status (searching, idle, detected).~~
    
- Allow the user to enable or disable the preview of the cameras from the tray icon.

- Remove curl ouput from console.

- Configuration:
    ~~-- Allow the user to disable / enable a camera.~~-
    ~~-- Allow the user to set a telegram bot API key.~~-
    - Allow the user to set a messaging service like dweet.io

## New way to read the configuration:
1. Read a scheduler.ini file that has the following format
`FROM: HH:MM TO: HH:MM FILE: config1.ini
FROM: HH:MM TO: HH:MM FILE: config2.ini
...
FROM: HH:MM TO: HH:MM FILE: confign.ini
`
2. At start read the corresponding config file and save the end time (TO)
3. Invoke a new thread with a method that waits the remaining time from now until TO. This could be implemented with a sleep, but no directly. I mean, you need to awake the thread every now and then to see if the flag "stop" is set.
4. When the method of (3) come backd from the sleep read the next configuration and go to (3).
