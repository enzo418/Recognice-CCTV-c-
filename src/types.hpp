#pragma once

#include "Recognice-CCTV-c-/src/types.hpp"
#include "Recognice-CCTV-c-/src/ConfigurationFile.hpp"
#include "Recognice-CCTV-c-/src/recognize.hpp"

#include <wx/checkbox.h>

struct SharedData {
	Recognize* recognize = nullptr;
	Configuration* configuration = nullptr;

	wxCheckBox* chckRecognizeActive = nullptr;
	bool* recognizeActive;
};