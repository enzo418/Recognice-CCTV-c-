#include "AreaSelector.hpp"

namespace AreaSelector {
	bool GetFrame(const std::string& url, cv::Mat& frame) {
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

	void ResizeFrameToCommon(cv::Mat& frame) {
		cv::resize(frame, frame, RESIZERESOLUTION);
	}

	bool SelectCameraROI(CameraConfiguration& cfg) {
		AreaDataROI data;
		bool success = false;

		if (GetFrame(cfg.url, data.frame)) {
			// setup window
			cv::namedWindow("Press a key to exit");
			cv::setMouseCallback("Press a key to exit", onMouseROI, &data);
			
			data.roi = cfg.roi;
			
			data.startPoint = cfg.roi.tl();
			
			// resize frame and draw current roi
			cv::resize(data.frame, data.frame, RESIZERESOLUTION);
			ImageManipulation::RotateImage(data.frame, cfg.rotation);
			data.frame.copyTo(data.show);
			cv::rectangle(data.show, cfg.roi, cv::Scalar(255, 25, 255), 2);
			
			cv::imshow("Press a key to exit", data.show);
			cv::waitKey(0);
			
			cv::destroyAllWindows();
			
			if (data.roi.width > RESIZERESOLUTION.width)
				data.roi.width = RESIZERESOLUTION.width;
				
			if (data.roi.height > RESIZERESOLUTION.height)
				data.roi.height = RESIZERESOLUTION.height;
			
			cfg.roi = data.roi;
			
			success = true;
		}

		return success;
	}

	void onMouseROI(int event, int x, int y, int flags, void* params) {
		AreaDataROI* areaSelector = reinterpret_cast<AreaDataROI*>(params);
		
		cv::Point point(x, y);
		bool updateImg = false;

		if (event == cv::EVENT_LBUTTONDOWN) {
			areaSelector->lastEvent = cv::EVENT_LBUTTONDOWN;
			areaSelector->startPoint = point;
		} else if (event == cv::EVENT_RBUTTONDOWN) {
			areaSelector->lastEvent = cv::EVENT_RBUTTONDOWN;
		} else if (event == cv::EVENT_LBUTTONUP || event == cv::EVENT_RBUTTONUP){
			areaSelector->lastEvent = -1;
		} else if (event == cv::EVENT_MOUSEMOVE) {
			if (areaSelector->lastEvent != -1){
				cv::Point point1 = areaSelector->startPoint;
				cv::Scalar color(255, 25, 255);
				areaSelector->roi = cv::Rect(point, point1);
				
				areaSelector->frame.copyTo(areaSelector->show);

				cv::rectangle(areaSelector->show, areaSelector->roi, color, 2);
				updateImg = true;
			}
		}

		if(updateImg) {
			cv::imshow("Press a key to exit", areaSelector->show);
		}
	}

	bool SelectCameraIgnoredAreas(CameraConfiguration& cfg) {
		AreaDataIgnoredAreas data;
		data.rng = cv::RNG(12345);
		bool success = false;
		
		if (GetFrame(cfg.url, data.frame)) {
			// setup window
			cv::namedWindow("Press a key to exit");
			cv::setMouseCallback("Press a key to exit", onMouseIgnoredAreas, &data);
			
			data.areas = &cfg.ignoredAreas;
			
			// resize, crop frame and draw current roi
			cv::resize(data.frame, data.frame, RESIZERESOLUTION);
			
			// crop
			if (!cfg.roi.empty())
				data.frame = data.frame(cfg.roi);
			
			// rotate
			ImageManipulation::RotateImage(data.frame, cfg.rotation);
				
			// copy original to drawable
			data.frame.copyTo(data.show);
			
			// draw ignored areas and add random colors
			for (auto&& area : cfg.ignoredAreas) {
				cv::Scalar color(data.rng.uniform(0, 256), data.rng.uniform(0, 256), data.rng.uniform(0, 256));
				data.colors.push_back(color);
				cv::rectangle(data.show, area, color, 2);
			}
			
			cv::imshow("Press a key to exit", data.show);
			cv::waitKey(0);
			
			cv::destroyAllWindows();

			success = true;
		}

		return success;
	}

	void onMouseIgnoredAreas(int event, int x, int y, int flags, void* params) {
		AreaDataIgnoredAreas* data = reinterpret_cast<AreaDataIgnoredAreas*>(params);
		
		cv::Point point(x, y);
		bool updateImg = false;

		if (event == cv::EVENT_LBUTTONDOWN){
			data->lastEvent = cv::EVENT_LBUTTONDOWN;
			
			data->areas->push_back(cv::Rect(point, cv::Size(1,1)));
			data->colors.push_back(cv::Scalar(data->rng.uniform(0, 256), data->rng.uniform(0, 256), data->rng.uniform(0, 256)));
		} else if (event == cv::EVENT_RBUTTONDOWN){
			data->lastEvent = -1;
			
			// delete the areas that contains the clicked point
			size_t sz = data->areas->size();
			for (size_t i = 0; i < sz; i++) {
				if ((*data->areas)[i].contains(point)) {
					data->areas->erase(data->areas->begin() + i);
					data->colors.erase(data->colors.begin() + i);
					sz--;
					i--;
					updateImg = true;
				}
			}
		} else if (event == cv::EVENT_LBUTTONUP || event == cv::EVENT_RBUTTONUP){
			data->lastEvent = -1;
		} else if (event == cv::EVENT_MOUSEMOVE){
			if (data->lastEvent != -1){
				data->areas->back() = cv::Rect(data->areas->back().tl(), point);
				
				updateImg = true;
			}
		}

		if(updateImg) {
			// clear drawed rectangles
			data->frame.copyTo(data->show);
			
			for (size_t i = 0; i < data->areas->size(); i++) {
				cv::rectangle(data->show, (*data->areas)[i], data->colors[i], 2);
			}
			
			cv::imshow("Press a key to exit", data->show);
		}
	}
}