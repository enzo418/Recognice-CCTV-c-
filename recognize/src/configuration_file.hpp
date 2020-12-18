#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdlib>

#ifdef WINDOWS
#include <Windows.h>
#include <shlwapi.h>
#else
#define strcpy_s(x,y) strcpy(x,y)
#define MAX_PATH 260
#endif

#include "types.hpp"
#include "types_configuration.hpp"
#include "utils.hpp"

namespace ConfigurationFile {
	/**
	 * @brief Get around for windows calls
	 * @param fileName
	 */
	 const char* GetFilePath(const char* fileName);

	 void OpenFileWrite(std::fstream& file, std::string& fn);
	 void OpenFileRead(std::fstream& file, std::string& fn);
	
	 void WriteConfigurationFileHeader(std::fstream& file);
	 void WriteConfiguration(std::fstream& file, CameraConfiguration& cfg);
	 void WriteConfiguration(std::fstream& file, ProgramConfiguration& cfg);

	template<typename T>
	 T ReadNextConfiguration(std::fstream& file, T& config);

	// writes the value in the field id
	 bool SaveIdVal(CameraConfiguration& config, std::string id, std::string value);

	// writes the value in the field id
	 bool SaveIdVal(ProgramConfiguration& config, std::string id, std::string value);

	 inline void WriteLineInFile(std::fstream& file, const char* line);

	 void PreprocessConfigurations(Configurations& cfgs);

	/**
	 * @brief Reads a file configuration and returns it
	 * @param fileName Optinal, leave if you want to use the last file used.
	 */
	 Configurations ReadConfigurations(std::string filePath);
	
	/**
	 * @brief Writes the configuration into a file
	 * @param cfgs Configurations
	 * @param fileName Optional, if it's blank the will use the last file name.
	 */
	 void SaveConfigurations(Configurations& cfgs, std::string filePath);
};