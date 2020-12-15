#pragma once

#include "../recognize/src/types.hpp"
#include "../recognize/src/configuration_file.hpp"
#include "../recognize/src/recognize.hpp"

#include <wx/checkbox.h>

struct SharedData {
	Recognize* recognize = nullptr;
	Configuration* configuration = nullptr;

	wxCheckBox* chckRecognizeActive = nullptr;
	bool* recognizeActive;
};