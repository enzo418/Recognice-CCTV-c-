#pragma once

#include <wx/frame.h>

#include "../recognize/src/types.hpp"
#include "../recognize/src/configuration_file.hpp"
#include "../recognize/src/recognize.hpp"

class cPreviewCameras : public wxFrame {
	public:
		cPreviewCameras(Recognize& rRecognize, Configurations& config, bool& recognizeActive, bool& mainClosed);
		~cPreviewCameras();

	private:
		Recognize* pRecognize;

};