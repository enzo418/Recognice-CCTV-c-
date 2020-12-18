#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>
#include <wx/msgdlg.h>
#include <chrono>
#include "../recognize/src/types.hpp"

namespace AreaSelector {
	struct AreaData {
		cv::Rect roi;
		int lastEvent = -1;
		cv::Point startPoint;
		cv::Mat frame;
		cv::Mat show;
	};
	
	/**
	 * @brief Callback for SelectCameraROI 
	 * @param event
	 * @param x
	 * @param y
	 * @param flags
	 * @param params
	 */
	void onMouseROI(int event, int x, int y, int flags, void* params);
	
	/**
	 * @brief Captures a frame from the url and returns true if sucess
	 * @param url source
	 * @param frame frame captures
	 */
	bool GetFrame(const std::string& url, cv::Mat& frame);
	
	/**
	 * @brief Shows a image captured from the url and let the user select a rectangle
	 * @param url source
	 * @param roi reference param
	 */
	void SelectCameraROI(const std::string& url, cv::Rect& roi);
};

