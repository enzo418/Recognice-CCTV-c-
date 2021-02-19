#pragma once
#include <opencv2/opencv.hpp>

#include "types.hpp"
#include "types_configuration.hpp"
#include "image_manipulation.hpp"
#include "utils.hpp"

#include <iostream>
#include <thread>
#include <vector>
#include <iterator>
#include <algorithm>
#include <queue>
#include <chrono>
#include <ctime>
#include <unordered_map> // To get a unique file id from a camera url
#include <memory>

#include "notification.hpp"
#include "gif_frames.hpp"

#include "readerwriterqueue.h" // lock-free queue, home page: https://github.com/cameron314/readerwriterqueue

class Camera {
private:
	// ============================
	//  Pointers to externals vars
	// =============================
	ProgramConfiguration* _programConfig = nullptr;
	cv::HOGDescriptor* _descriptor = nullptr;

	// ============
	//  Class vars
	// ============
	cv::VideoCapture capturer;
	
	ulong msBetweenFrames;
	
	// current frame.
	cv::Mat frame;

	// at the end of the processing of the current frame it is stored in lastFrame to be compared with the next one.
	cv::Mat lastFrame;

	// frame to send to the display.
	cv::Mat frameToShow;
		
	std::unique_ptr<GifFrames> currentGifFrames;
	
	// used to store the diff frame between lastFrame and frame.
	cv::Mat diff; 

	// Rectangle that causes incorrect detections
	std::vector<FindingInfo> untFindings;
	
	std::chrono::time_point<std::chrono::high_resolution_clock> lastPersonDetected = std::chrono::high_resolution_clock::now();

	int totalNonZeroPixels = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now(), 
								lastSavedImage = std::chrono::high_resolution_clock::now();
	
	ushort numberFramesToAdd = 0;
	ushort maxFramesLeft = 0;
	ushort framesLeft = 0;

	// ==================
	//  Change-threshold
	// ==================

	// the update Frequency of the threshold in seconds
	const int updateFrequencyThreshold = 5;

	// accumulator used while
	unsigned long accumulatorThresholds = 0;
	
	// how many samples were collected
	uint64_t thresholdSamples = 0;

	// the updatable threshold
	int temporalChangeThreshold = 0;
	
	// this will be used as indicator to know how much the threshold will be from the average threshold
	const double abovePercentage = 1.2;

	// saves the last time the threshold was updated
	std::chrono::time_point<std::chrono::high_resolution_clock> lastThresholdUpdate = std::chrono::high_resolution_clock::now();

	FindingInfo lastFinding;

	cv::VideoWriter outVideo;

	void UpdateThreshold();

	// ==================
	//  Methods
	// ==================

	void ReadFramesWithInterval();

	void CalculateNonZeroPixels();

	void ApplyBasicsTransformations();

	void OpenVideoWriter();

	void ChangeTheStateAndAlert(std::chrono::time_point<std::chrono::high_resolution_clock>& now);

public:
	Camera(CameraConfiguration& cameraConfig, ProgramConfiguration* programConfig, cv::HOGDescriptor* hog);
	~Camera();

	void ReleaseChangeVideo();

	void AppendFrameToVideo(cv::Mat& frame);
	
public:
	std::chrono::time_point<std::chrono::high_resolution_clock>
								lastImageSended = std::chrono::high_resolution_clock::now(),
								lastTextSended = std::chrono::high_resolution_clock::now();

	bool close = false;
	
	std::unique_ptr<std::thread> cameraThread;

	CameraConfiguration* config = nullptr;

	// alerts created by this camera
	std::vector<Notification::Notification> pendingNotifications;

	// List of frames captured
	std::unique_ptr<moodycamel::ReaderWriterQueue<cv::Mat>> frames;

	// Temporal list of registers of when someone did enter o leave a site.
	std::vector<Register> registers;

	std::vector<std::unique_ptr<GifFrames>> gifsReady;
	
	// Current state of the camera sentry, detecting or detected.
	NISTATE state;

	void Connect();	
};