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

	void ReadFramesWithInterval();

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