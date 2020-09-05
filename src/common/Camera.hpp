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
	/** Externals vars */
	bool* _stop_flag = nullptr;
	ProgramConfiguration* _programConfig = nullptr;	
	cv::HOGDescriptor* _descriptor = nullptr;

	/** Class vars */
	cv::VideoCapture capturer;
	
	// current frame.
	cv::Mat frame;

	// at the end of the processing of the current frame it is stored in lastFrame to be compared with the next one.
	cv::Mat lastFrame;

	// frame to send to the display.
	cv::Mat frameToShow;
	
	// used to store the diff frame between lastFrame and frame.
	cv::Mat diff; 

	int totalNonZeroPixels = 0;

	std::chrono::system_clock::time_point now = std::chrono::high_resolution_clock::now(), 
								lastSavedImage = std::chrono::high_resolution_clock::now(),
								lastMessageSended = std::chrono::high_resolution_clock::now();

	std::vector<cv::Rect> detections;
	std::vector< double > foundWeights;


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

	Camera(CameraConfiguration cameraConfig, ProgramConfiguration* programConfig, bool* stopFlag, cv::HOGDescriptor* hog);

	void Connect();

	// Creates a thread which calls to a internal method and then return the thread.
	std::thread StartDetection();
	
};