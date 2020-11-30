#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

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

#include "ImageManipulation.hpp"
#include "utils.hpp"
#include "ConfigurationFile.hpp"

#include "TelegramBot.hpp"

#ifdef WINDOWS
#else
#define sprintf_s sprintf
#endif
#include "Camera.hpp"

#define RECOGNICEALWAYS false
#define SHOWFRAMEINSCREEN true

//using namespace cv; // Gives error whe used with <Windows.h>
using namespace std::chrono;

struct FrameDescriptor {
	cv::Mat frame;
	FindingInfo finding;
};

class Recognize {
private:
	size_t indexMainThreadCameras;

	std::vector<Camera> cameras;

	ProgramConfiguration programConfig;
	CamerasConfigurations camerasConfigs;

	cv::HOGDescriptor hogDescriptor;

	std::vector<cv::Mat*> AnalizeLastFramesSearchBugs(Camera& camera);
public:
	bool stop = false;
	bool close = false;

	// use this variable to join all the threads when want to destroy this instantiation
	std::vector<std::thread> threads;

	Recognize(void);

	void Start(Configurations configs, bool startPreviewThread, bool startActionsThread);

	void StartActionsBot();

	void StartNotificationsSender();

	void StartPreviewCameras();
	
	void StartCamerasThreads();
};