#pragma once

#include <iostream>
#include "../recognize/src/types.hpp"
#include "../recognize/src/configuration_file.hpp"
#include "../recognize/src/recognize.hpp"

#include <wx/checkbox.h>

struct SharedData {
	Recognize* recognize = nullptr;
	Configurations* configurations = nullptr;

	wxButton* btnApplyChanges;
	bool* recognizeActive;
};