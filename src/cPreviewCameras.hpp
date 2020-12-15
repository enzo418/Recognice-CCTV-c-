#pragma once

#include <wx/frame.h>

#include "Recognice-CCTV-c-/src/types.hpp"
#include "Recognice-CCTV-c-/src/ConfigurationFile.hpp"
#include "Recognice-CCTV-c-/src/recognize.hpp"

class cPreviewCameras : public wxFrame {
	public:
		cPreviewCameras(Recognize& rRecognize, Configurations& config, bool& recognizeActive, bool& mainClosed);
		~cPreviewCameras();

	private:
		Recognize* pRecognize;

};