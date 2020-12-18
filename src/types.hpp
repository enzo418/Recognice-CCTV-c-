#pragma once

#include "../recognize/src/types.hpp"
#include "../recognize/src/configuration_file.hpp"
#include "../recognize/src/recognize.hpp"

#include <wx/checkbox.h>

struct SharedData {
	Recognize* recognize = nullptr;
	Configurations* configurations = nullptr;

	wxCheckBox* chckRecognizeActive = nullptr;
	bool* recognizeActive;
};