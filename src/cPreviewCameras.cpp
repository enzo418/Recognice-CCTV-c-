#include "cPreviewCameras.hpp"
#include <opencv2/core.hpp>

cPreviewCameras::cPreviewCameras(Recognize& rRecognize, Configurations& config, bool& recognizeActive, bool& mainClosed) 
	: 	wxFrame(nullptr, wxID_ANY, "Preview", wxPoint(30, 30), wxSize(840, 480)),
		pRecognize(&rRecognize) {	
	bool f = false;
	while (!mainClosed) {
		if (recognizeActive && config.programConfig.showPreview) {
			pRecognize->StartPreviewCameras();
		} else {
			if (!f) {
				cv::namedWindow("Dummy");
				cv::startWindowThread();
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				cv::destroyAllWindows();
				f = true;
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
	}	
	this->Close(true);
}

cPreviewCameras::~cPreviewCameras() {

}