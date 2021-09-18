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
#include <memory>
#include <tuple>
#include <filesystem>

#include "image_manipulation.hpp"
#include "utils.hpp"
#include "configuration_file.hpp"

#include "telegram_bot.hpp"

#include "readerwriterqueue.h" // lock-free queue, home page: https://github.com/cameron314/readerwriterqueue

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
	
	DetectionMethod lastDetectionMethod = DetectionMethod::DoNotUse;
	
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

	// keep a record of the notifications sended with media
	std::unique_ptr<moodycamel::ReaderWriterQueue<std::tuple<Notification::Type, std::string, std::string, std::string>>> notificationWithMedia;

	Recognize(void);
	~Recognize() = default;

	bool Start(const Configurations& configs, bool startPreviewThread, bool startActionsThread, std::string& error);

	void StartActionsBot();

	void StartNotificationsSender();

	void StartPreviewCameras();
	
	void StartCamerasThreads();

	// waits until all the threads are joined
	// then returns
	void CloseAndJoin();
};