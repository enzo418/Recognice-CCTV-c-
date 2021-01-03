#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/all_layers.hpp>

#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <thread>
#include <map>
#include <chrono>
#include <ctime>
#include <sys/stat.h> // To check if file exist
#include <unordered_map> // To get a unique file id from a camera url
#include <signal.h> // to catch ctrl c signal

#include "image_manipulation.hpp"
#include "utils.hpp"
#include "configuration_file.hpp"

#include "telegram_bot.hpp"

#ifdef WINDOWS
#else
#define sprintf_s sprintf
#endif
#include "camera.hpp"

#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std::chrono;

class Recognize {
private:
	size_t indexMainThreadCameras;

	ProgramConfiguration programConfig;
	CamerasConfigurations camerasConfigs;

	cv::HOGDescriptor* hogDescriptor;

	std::vector<cv::Mat*> AnalizeLastFramesSearchBugs(Camera& camera);
	
	DetectionMethod lastDetectionMethod = DetectionMethod::None;
	
	cv::dnn::Net* net;
	std::vector<std::string> output_names;
	std::vector<std::string> class_names;
	int num_classes;
	const float CONFIDENCE_THRESHOLD = 0;
	const float NMS_THRESHOLD = 0.4;
	
	std::vector<std::tuple<cv::Rect, double, std::string>> Detect(cv::Mat& frame, CameraConfiguration& cfg);
public:
	bool stop = false;
	bool close = false;

	// use this variable to join all the threads when want to destroy this instantiation
	std::vector<std::thread> threads;

	std::vector<std::unique_ptr<Camera>> cameras;

	Recognize(void);
	~Recognize() = default;

	void Start(Configurations& configs, bool startPreviewThread, bool startActionsThread);

	void StartActionsBot();

	void StartNotificationsSender();

	void StartPreviewCameras();
	
	void StartCamerasThreads();

	void CloseAndJoin();
};