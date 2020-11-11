#pragma once
#include <opencv2/opencv.hpp>

#include "types.hpp"
#include "types_configuration.hpp"
#include "ImageManipulation.hpp"
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

class Camera {
private:
	// ============================
	//  Pointers to externals vars
	// =============================
	bool* _stop_flag = nullptr;
	ProgramConfiguration* _programConfig = nullptr;	
	cv::HOGDescriptor* _descriptor = nullptr;

	// ============
	//  Class vars
	// ============
	cv::VideoCapture capturer;
	
	// current frame.
	cv::Mat frame;

	// at the end of the processing of the current frame it is stored in lastFrame to be compared with the next one.
	cv::Mat lastFrame;

	// frame to send to the display.
	cv::Mat frameToShow;
	
	// used to store the diff frame between lastFrame and frame.
	cv::Mat diff; 

	// Rectangle that causes incorrect detections
	std::vector<FindingInfo> untFindings;
	
	// Used to save the finding as untrusted if no person was detected
	bool personDetected = false;

	// The diff frame that caused the start of detection
	cv::Mat diffFrameCausedDetection;

	int totalNonZeroPixels = 0;

	std::chrono::system_clock::time_point now = std::chrono::high_resolution_clock::now(), 
								lastSavedImage = std::chrono::high_resolution_clock::now(),
								lastMessageSended = std::chrono::high_resolution_clock::now();
								// lastBackupImageStored = std::chrono::high_resolution_clock::now();

	std::vector<cv::Rect> detections;
	std::vector< double > foundWeights;

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
	std::chrono::system_clock::time_point lastThresholdUpdate = std::chrono::high_resolution_clock::now();

	void UpdateThreshold();

	// ==================
	//  Methods
	// ==================

	void ReadFramesWithInterval();

	void CalculateNonZeroPixels();

	void ApplyBasicsTransformations();

	void ChangeTheStateAndAlert(std::chrono::system_clock::time_point& now);

	void CheckForHumans();

public:
	CameraConfiguration config;

	// alerts created by this camera
	std::vector<Message> pendingAlerts;

	// List of frames captured
	std::vector<cv::Mat> frames;

	// Temporal list of registers of when someone did enter o leave a site.
	std::vector<Register> registers;

	// that
	// cv::Mat imageFrom10SecondsAgo;

	Camera(CameraConfiguration cameraConfig, ProgramConfiguration* programConfig, bool* stopFlag, cv::HOGDescriptor* hog);

	void Connect();

	// Creates a thread which calls to a internal method and then return the thread.
	std::thread StartDetection();
	
};