#include "Configuration.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

// This module is not complete.

cv::Mat img;
cv::Mat dst;
cv::Point entryPoint1;
cv::Point entryPoint2;
cv::Rect lastRectEntry;

cv::Point exitPoint1;
cv::Point exitPoint2;
cv::Rect lastRectExit;

CameraConfig config;
int lastEvent = -1;

int radius = 50;
cv::Scalar colorEntry(0,255,255);
cv::Scalar colorExit(0,255,0);

static void onMouse( int event, int x, int y, int flags, void* params) {
    cv::Point point(x, y);
    dst = img.clone();
    bool updateImg = false;

    if (event == cv::EVENT_LBUTTONDOWN){ // EVENT_LBUTTONDOWN
        lastEvent = cv::EVENT_LBUTTONDOWN;        
        entryPoint1 = point;
    } else if (event == cv::EVENT_RBUTTONDOWN){ // EVENT_RBUTTONDOWN
        lastEvent = cv::EVENT_RBUTTONDOWN;
        exitPoint1 = point;
    } else if (event == cv::EVENT_LBUTTONUP || event == cv::EVENT_RBUTTONUP){
        lastEvent = -1;
        updateImg = false;
    } else if (event == cv::EVENT_MOUSEMOVE){
        if (lastEvent != -1){
            cv::Point point1 = lastEvent == cv::EVENT_RBUTTONDOWN ? exitPoint1 : entryPoint1;
            cv::Scalar color;
            cv::Rect rect = cv::Rect(cv::Point(point1.x - point.x / 2, point1.y - point.y / 2), cv::Size(point));

            if (lastEvent == cv::EVENT_RBUTTONDOWN){
                exitPoint2 = point;
                color = colorExit;
                lastRectExit = rect;
                if(!lastRectEntry.empty()) cv::rectangle(dst, lastRectEntry, colorEntry, radius/10);
            } else if (lastEvent == cv::EVENT_LBUTTONDOWN){
                entryPoint2 = point;
                color = colorEntry;
                lastRectEntry = rect;
                if(!lastRectExit.empty()) cv::rectangle(dst, lastRectExit, colorExit, radius/10);
            }

            cv::rectangle(dst, rect, color, radius/10);
            updateImg = true;
        }
    }

    if(updateImg){
        cv::imshow("Press a key to exit", dst);
    }
}

AreasDelimiters StartConfigurationAreaEntryExit(CameraConfig& config){

    cv::namedWindow("Press a key to exit", 0);

    cv::setMouseCallback("Press a key to exit", onMouse, 0);  

    cv::VideoCapture capture(config.url);
    capture.read(img);

    //cv::resize(img, img, RESIZERESOLUTION);

    if (!config.roi.isEmpty()) {
        cv::Rect roi(config.roi.point1, config.roi.point2);
        img = img(roi);
    }
    
    cv::imshow("Press a key to exit", img);

    cv::waitKey(0);

    return {lastRectEntry, lastRectExit};
}

void LoadConfigCamera(CameraConfig& src, CameraConfig& dst, bool isModification = false) {
	std::string input;        
	
	if (isModification)
		std::cout << "\tNote: if you want to leave te original value [value], then just press enter when the program ask you to enter it."
		<< std::endl;

	while (1) {
		std::cout << "Enter the camera name: ";
		if (isModification) std::cout << "[" << src.cameraName << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.cameraName = input;

		std::cout << "Enter the camera url: ";
		if (isModification) std::cout << "[" << src.url << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.url = input;

		std::cout << "Enter the camera roi";
		if (isModification) std::cout << "[" << Utils::RoiToString(src.roi) << "] ";
		else std::cout << ", format [(x1,y1),(x2,y2)]: ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.roi = Utils::GetROI(input);

		std::cout << "Enter camera order: ";
		if (isModification) std::cout << "[" << src.order << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.order = std::stoi(input);

		std::cout << "Enter rotation (deg): ";
		if (isModification) std::cout << "[" << src.rotation << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.rotation = std::stoi(input);

		std::cout << "Enter camera change threshold: ";
		if (isModification) std::cout << "[" << src.changeThreshold << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			dst.changeThreshold = std::stoi(input);

		std::cout << "Enter the camera type Sentry, Active, Disabled: ";
		if (isModification) std::cout << "[" << Utils::CamTypeToString(src.type) << "] ";
		std::getline(std::cin, input);
		Utils::toLowerCase(input);
		if (input == "sentry") {
			dst.type = CAMERA_SENTRY;
		} else if (input == "disabled") {
			dst.type = CAMERA_DISABLED;
		} else {
			dst.type = CAMERA_ACTIVE;
		}

		std::cout << "Enter the hit threshold (0-1): ";
		if (isModification) std::cout << "[" << src.hitThreshold << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			Config::SaveIdVal(dst, "hitthreshold", input);

		std::cout << "Enter the noise threshold: ";
		if (isModification) std::cout << "[" << src.noiseThreshold << "] ";
		std::getline(std::cin, input);
		if (!input.empty())
			Config::SaveIdVal(dst, "noisethreshold", input);
				
		std::cout << "The program will show you a window where you will nedd to select: "
						"\n The area where if a person is seen in it, they will be marked as entering the site."
						"\n The area where if a person is seen in it, they will be marked as leaving the site."
						"Use LeftMouseClick or LeftMouseClick to set the set the entry or exit point and then Move the mouse to select the area.";
		// call the f
		dst.areasDelimiters = StartConfigurationAreaEntryExit(dst);

		std::cout << "Exit and save changes? (yes/no):";
		std::getline(std::cin, input);
		if (input == "yes" || input == "YES") {
			src = dst;			
			return;
		} else {
			std::cout << "Edit again? (yes/no):";
			std::getline(std::cin, input);
			if (input == "no" || input == "NO") return;
		}
	}
}

void Config::ModifyCamera(std::vector<CameraConfig>& configs) {
	std::cout << "\n# Modify camera config\n";
	std::string input;

	size_t size = configs.size();

	for (size_t i = 0; i < size; i++) {
		std::cout << "\t" << i+1 << ". " << configs[i].cameraName << std::endl;
		std::cout << "\t  " << "url=" << configs[i].url << std::endl;
	}

	while (input.empty()) {
		std::cout << "- Please enter a camera config:";
		std::getline(std::cin, input);
	}

	int indx = std::stoi(input) - 1;

	if (indx >= 0 && indx < size) {
		LoadConfigCamera(configs[indx], configs[indx], true);
		// Write config in file
	} else std::cout << "ERROR: Invalid index " << indx << std::endl;
}

void Config::AddNewCamera(std::vector<CameraConfig>& configs) {
	CameraConfig config;

	LoadConfigCamera(config, config, false);

	Config::File::AppendCameraConfig(config);

	if (config.type != CAMERA_DISABLED) configs.push_back(config);
}

void Config::CameraConfiguration(std::vector<CameraConfig>& configs) {
	std::string input;
	while (input.empty()) {
		std::cout << "\n# Cameras configurations" << std::endl
			<< "\t1. Add new camera" << std::endl
			<< "\t2. Modify camera" << std::endl
			<< "- Chose a option: ";

		std::getline(std::cin, input);
	}

	if (input == "1") {
		Config::AddNewCamera(configs);
	} else if (input == "2") {
		Config::ModifyCamera(configs);
	}
}

void Config::ProgramConfiguration(ProgramConfig& config) {

}

void Config::StartConfiguration(std::vector<CameraConfig>& configs, ProgramConfig& programConfig) {
	std::string input;
	while (input.empty()) {
		std::cout << "# Configuration" << std::endl
			<< "\t1. Cameras" << std::endl
			<< "\t2. Program" << std::endl
			<< "- Chose a option: ";

		std::getline(std::cin, input);
	}

	if (input == "1") {
		Config::CameraConfiguration(configs);
	} else if (input == "2") {
		Config::ProgramConfiguration(programConfig);
	}
}
