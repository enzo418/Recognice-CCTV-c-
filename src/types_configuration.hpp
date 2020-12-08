#pragma once

#include "types.hpp"

// ===============
// 	   Types
// ===============

struct NumberFramesBeforeAfter {
	size_t* framesBefore = nullptr;
	size_t* framesAfter = nullptr;
};

struct CameraConfiguration {
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

	// Temporal list of registers of when someone did enter o leave a site.
	std::vector<Register> registers;

	// Time to wait from the time a person reaches the point of entry or exit until they reach the other point
	int secondsWaitEntryExit;

	// threshold to use when removing the noise of the frame
	double noiseThreshold;

	// Delimiters to the entry vs exit area.
	AreasDelimiters areasDelimiters;

	// used as minimun value of threshold (to avoid noise)
	int minimumThreshold;

	// % to increase from the minimum threshold
	float increaseTresholdFactor;

	uint32_t updateThresholdFrequency;

	bool useHighConstrast = false;

	std::vector<cv::Rect> ignoredAreas;

	NumberFramesBeforeAfter framesToAnalyze;
};

// to be able to sort the array of configs
struct {
	inline bool operator() (const CameraConfiguration& struct1, const CameraConfiguration& struct2) {
		return (struct1.order < struct2.order);
	}
}less_than_order;

enum GifResizePercentage {None = 100, Low = 80, Medium = 60, High = 40, VeryHigh = 20};

struct ProgramConfiguration {
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

	// if should send a image with all the cameras when something is detected.
	bool sendImageOfAllCameras;

	// Used to save a frame with all the cameras, is updated every
	cv::Mat frameWithAllTheCameras;

	// holds a vector of users that are allowed to send commands to the bot
	std::vector<std::string> authUsersToSendActions;

	// % used to scale the output image
	double ratioScaleOutput;

	// relative folder to save the imgs of the changes / detections
	std::string imagesFolder = "saved_imgs";

	bool useGifInsteadImage = true;

	GifResizePercentage gifResizePercentage = GifResizePercentage::Medium;

	NumberFramesBeforeAfter numberGifFrames;
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