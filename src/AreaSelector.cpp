#include "AreaSelector.hpp"

AreaSelector::AreaSelector()
{
}

AreaSelector::~AreaSelector()
{
}


bool AreaSelector::GetFrame(const std::string& url, cv::Mat& frame) {
	bool sucess = false;
	
	cv::VideoCapture cap(url);
	
	if (cap.isOpened()) {
		// 500 * 50 ms = 2.5 s
		int tries = 500;
		while (!cap.read(frame) && tries > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			tries--;
		}
		
		if (tries > 0)
			sucess = true;
	}
	
	return sucess;
}

void AreaSelector::SelectCameraROI(const std::string& url, cv::Rect& roi) {
	if (this->GetFrame(url, this->m_data.frame)) {
		// setup window
		cv::namedWindow("Press a key to exit");
		cv::setMouseCallback("Press a key to exit", AreaSelector::onMouseROI, &this->m_data);
		
		this->m_data.startPoint = roi.tl();
		
		// crop frame and draw current roi
		cv::resize(this->m_data.frame, this->m_data.frame, RESIZERESOLUTION);
		this->m_data.frame.copyTo(this->m_data.show);
		cv::rectangle(this->m_data.show, roi, cv::Scalar(255, 25, 255), 2);
		
		cv::imshow("Press a key to exit", this->m_data.show);
		cv::waitKey(0);
		
		cv::destroyAllWindows();
		
		roi = this->m_data.roi;
	} else {
		wxMessageBox("Couldn't open the camera", "Error");
	}
}

void AreaSelector::onMouseROI(int event, int x, int y, int flags, void* params) {
	AreaData* areaSelector = reinterpret_cast<AreaData*>(params);
	
	cv::Point point(x, y);
    bool updateImg = false;

    if (event == cv::EVENT_LBUTTONDOWN){ // EVENT_LBUTTONDOWN
        areaSelector->lastEvent = cv::EVENT_LBUTTONDOWN;
        areaSelector->startPoint = point;
    } else if (event == cv::EVENT_RBUTTONDOWN){ // EVENT_RBUTTONDOWN
        areaSelector->lastEvent = cv::EVENT_RBUTTONDOWN;
//        areaSelector->exitPoint1 = point;
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
