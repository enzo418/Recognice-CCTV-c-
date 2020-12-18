#pragma once
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>
#include <wx/msgdlg.h>
#include <chrono>
#include "../recognize/src/types.hpp"

struct AreaData {
	cv::Rect roi;
	int lastEvent = -1;
	cv::Point startPoint;
	cv::Mat frame;
	cv::Mat show;
};

class AreaSelector {
private:
	AreaData m_data;

private:
	static void onMouseROI(int event, int x, int y, int flags, void* params);
	bool GetFrame(const std::string& url, cv::Mat& frame);
	
public:
	AreaSelector();
	~AreaSelector();
	
public:
	void SelectCameraROI(const std::string& url, cv::Rect& roi);

};

