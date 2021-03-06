#pragma once

#include "types.hpp"

// ===============
// 	   Types
// ===============

struct NumberFramesBeforeAfter {
	size_t framesBefore = -1;
	size_t framesAfter = -1;
};

struct CameraConfiguration {
	std::string cameraName = "new_camera";

	// url or path to the camera / video
	std::string url;

	// Region of interest
	cv::Rect roi;

	// Threshold to recognice that a match/detection is valid
	double hitThreshold = 0.05;

	// Used to show the cameras sorted by order
	int order = 0;

	// rotation of the camera
	int rotation = 0;

	// pixels that must change to change state to detecting
	int changeThreshold = 1000;

	// The type of the camera. See definition.
	CAMERATYPE type = CAMERA_DISABLED;

	// Time to wait from the time a person reaches the point of entry or exit until they reach the other point
	int secondsWaitEntryExit = 1;

	// threshold to use when removing the noise of the frame
	double noiseThreshold = 45;

	// Delimiters to the entry vs exit area.
	AreasDelimiters areasDelimiters;

	// used as minimun value of threshold (to avoid noise)
	int minimumThreshold = 2;

	// % to increase from the minimum threshold
	double increaseTresholdFactor = 1.2;

	uint32_t updateThresholdFrequency = 15;

	bool useHighConstrast = false;

	std::vector<cv::Rect> ignoredAreas;

	// number of frames to run the algorith to detect a person
	NumberFramesBeforeAfter framesToAnalyze;

	// total frames to ignore a finding that the change is inside an ignored area
	size_t thresholdFindingsOnIgnoredArea = 2;

	// how much % of the finding needs to be inside the ignored area to ignore it
	double minPercentageAreaNeededToIgnore = 95.0;

	// Discriminators of points (center of a finding) to determine if the change is valid or not
	std::vector<PointsDiscriminatorArea> pointDiscriminators;

	double minPercentageInsideAllowDiscriminator = 80;
	double maxPercentageInsideDenyDiscriminator = 10;
};

// to be able to sort the array of configs
struct {
	inline bool operator() (const CameraConfiguration& struct1, const CameraConfiguration& struct2) {
		return (struct1.order < struct2.order);
	}
}less_than_order;

enum DetectionMethod {DoNotUse = 0, HogDescriptor = 1, YoloDNN_V4};

enum DrawTraceOn {None = 0, Image, Gif, Video, All};

struct ProgramConfiguration {
	// milliseconds to wait until get a new frame from the camera
	ushort msBetweenFrame = 25;
	
	// same as msBetweenFrame but will be used only after change
	ushort msBetweenFrameAfterChange = 30;

	// seconds to wait until save a new frame to the fraesToUpload list of the camera.
	int secondsBetweenImage = 15;

	// seconds to waait until send another message.
	int secondsBetweenMessage = 15;

	// resolution of the preview
	cv::Size outputResolution;

	// telefram bot config
	TelegramBotConfig telegramConfig;

	LocalNotificationsConfig localNotificationsConfig;

	// show preview of the cameras
	bool showPreview;

	// if should show in the preview the area that the camera sees
	bool showAreaCameraSees;

	// if should show the processed images with applied roation, threshold, ...
	bool showProcessedFrames;

	// if should send a image with all the cameras when something is detected.
	bool sendImageOfAllCameras;

	// holds a vector of users that are allowed to send commands to the bot
	std::vector<std::string> authUsersToSendActions;

	// % used to scale the output image
	double ratioScaleOutput = 1.0;

	// relative folder to save the imgs of the changes / detections
	std::string imagesFolder = "saved_imgs";
	
	ushort gifResizePercentage = 60;

	NumberFramesBeforeAfter numberGifFrames;

	bool showIgnoredAreas = false;

	// Used to save a frame with all the cameras, is updated every
	cv::Mat frameWithAllTheCameras;
	
	DetectionMethod detectionMethod = DetectionMethod::HogDescriptor;

	bool analizeBeforeAfterChangeFrames = true;

	NumberFramesBeforeAfter framesToAnalyzeChangeValidity;

	bool saveChangeInVideo = false;

	bool drawChangeFoundBetweenFrames = false;

	std::string messageOnTextNotification = "Movement detected on camera {N}";

	/* If the program should draw on the notification the trace of the change found on: 
	/* 	- 0: none
	/*  - 1: image
	/*  - 2: gif
	/* 	- 3: video
	/*  - 4: all
	*/
	DrawTraceOn drawTraceOfChangeFoundOn = DrawTraceOn::All;
};

typedef std::vector<CameraConfiguration> CamerasConfigurations;

struct Configurations {
	ProgramConfiguration programConfig;
	CamerasConfigurations camerasConfigs;
};


struct AreaEntryExitConfig {
	cv::Mat img;
	cv::Mat dst;
	cv::Point entryPoint1;
	cv::Point entryPoint2;
	cv::Rect lastRectEntry;

	cv::Point exitPoint1;
	cv::Point exitPoint2;
	cv::Rect lastRectExit;

	CameraConfiguration config;
	int lastEvent = -1;

	int radius = 50;
	cv::Scalar colorEntry;
	cv::Scalar colorExit;
};