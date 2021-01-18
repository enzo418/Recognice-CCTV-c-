#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>
#include <chrono>
#include "../../recognize/src/utils.hpp"
#include "../../recognize/src/types.hpp"
#include "../../recognize/src/types_configuration.hpp"
#include "../../recognize/src/image_manipulation.hpp"

namespace AreaSelector {
	struct AreaDataROI {
		int lastEvent = -1;
		cv::Point startPoint;
		cv::Mat frame;
		cv::Mat show;
		
		cv::Rect roi;
	};
	
	struct AreaDataIgnoredAreas {
		int lastEvent = -1;
		cv::Mat frame;
		cv::Mat show;
		
		cv::RNG rng;
		std::vector<cv::Rect>* areas;
		std::vector<cv::Scalar> colors;
	};

	/**
	 * @brief Captures a frame from the url and returns true if sucess
	 * @param url source
	 * @param frame frame captures
	 */
	bool GetFrame(const std::string& url, cv::Mat& frame);
	
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
	 * @brief Shows a image captured from the url and let the user select a rectangle
	 * @param url source
	 * @param roi reference param
	 */
	bool SelectCameraROI(CameraConfiguration& cfg);
			
	/**
	 * @brief Callback for SelectCameraIgnoredAreas
	 * @param event
	 * @param x
	 * @param y
	 * @param flags
	 * @param params
	 */
	void onMouseIgnoredAreas(int event, int x, int y, int flags, void* params);
	
	/**
	 * @brief Shows a image captured from the url and let the user select several rectangles
	 * @param url source
	 * @param roi reference param
	 */
	bool SelectCameraIgnoredAreas(CameraConfiguration& cfg);

	/**
	 * @brief Helper function to resize a frame to the common resolution
	 * @param frame frame to resize
	 */
	void ResizeFrameToCommon(cv::Mat& frame);
};

