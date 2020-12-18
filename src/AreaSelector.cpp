#include "AreaSelector.hpp"

bool AreaSelector::GetFrame(const std::string& url, cv::Mat& frame) {
	bool sucess = false;
	
	cv::VideoCapture cap(url);
	
	if (cap.isOpened()) {
		// 500 * 50 ms = 2.5 s
		int tries = 500;
		while (cap.isOpened() && !cap.read(frame) && tries > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			tries--;
		}
		
		if (tries > 0)
			sucess = true;
	}
	
	return sucess;
}

void AreaSelector::SelectCameraROI(const std::string& url, cv::Rect& roi) {
	AreaData data;
	
	if (GetFrame(url, data.frame)) {
		// setup window
		cv::namedWindow("Press a key to exit");
		cv::setMouseCallback("Press a key to exit", AreaSelector::onMouseROI, &data);
		
		data.startPoint = roi.tl();
		
		// crop frame and draw current roi
		cv::resize(data.frame, data.frame, RESIZERESOLUTION);
		data.frame.copyTo(data.show);
		cv::rectangle(data.show, roi, cv::Scalar(255, 25, 255), 2);
		
		cv::imshow("Press a key to exit", data.show);
		cv::waitKey(0);
		
		cv::destroyAllWindows();
		
		roi = data.roi;
	} else {
		wxMessageBox("Couldn't open the camera", "Error");
	}
}

void AreaSelector::onMouseROI(int event, int x, int y, int flags, void* params) {
	AreaData* areaSelector = reinterpret_cast<AreaData*>(params);
	
	cv::Point point(x, y);
    bool updateImg = false;

    if (event == cv::EVENT_LBUTTONDOWN){
        areaSelector->lastEvent = cv::EVENT_LBUTTONDOWN;
        areaSelector->startPoint = point;
    } else if (event == cv::EVENT_RBUTTONDOWN){
        areaSelector->lastEvent = cv::EVENT_RBUTTONDOWN;
    } else if (event == cv::EVENT_LBUTTONUP || event == cv::EVENT_RBUTTONUP){
        areaSelector->lastEvent = -1;
    } else if (event == cv::EVENT_MOUSEMOVE){
        if (areaSelector->lastEvent != -1){
            cv::Point point1 = areaSelector->startPoint;
            cv::Scalar color(255, 25, 255);
            areaSelector->roi = cv::Rect(point, point1);
			
			areaSelector->frame.copyTo(areaSelector->show);

            cv::rectangle(areaSelector->show, areaSelector->roi, color, 2);
            updateImg = true;
        }
    }

    if(updateImg){
        cv::imshow("Press a key to exit", areaSelector->show);
    }
}
