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

class Configuration {
private:
	const char* _fileName = "./config.ini";
	std::fstream _file;
	char openMode = '\0';

	AreaEntryExitConfig* areaConfig; // used only as a temporal way to store some data for StartConfigurationAreaEntryExit

	const char* GetFilePath(const char* fileName);

	void OpenFile();
	inline void CloseFile();

	void WriteConfigurationFileHeader();
	void WriteConfiguration(CameraConfiguration& cfg);
	void WriteConfiguration(ProgramConfiguration& cfg);

	template<typename T>
	T ReadNextConfiguration(std::fstream& file, T& config);

	// writes the value in the field id
	void SaveIdVal(CameraConfiguration& config, std::string id, std::string value);

	// writes the value in the field id
	void SaveIdVal(ProgramConfiguration& config, std::string id, std::string value);

	/// --- Proc/funcs to create or modify configurations
	void StartCameraConfiguration();
	void ReadNewCamera();
	void LoadConfigCamera(CameraConfiguration& src, CameraConfiguration& dst, bool isModification);
	AreasDelimiters StartConfigurationAreaEntryExit(CameraConfiguration& config);
	void SetAreaDelimitersCamera();
	static void onMouse(int event, int x, int y, int flags, void* params);
	void StartProgramConfiguration();

	inline void WriteLineInFile(const char* line);

	void PreprocessConfigurations();

public:
	Configurations configurations;

	/// ---- Constructors
	Configuration();

	Configuration(const char* filePath);

	/// ---- Other Public messages

	/// <summary> Reads the config file </summary>
	void ReadConfigurations();

	/// <summary> Interacts with the user to add or modificate configurations </summary>
	void StartConfiguration();
	
	/// <summary> Writes the configurations into the file </summary>
	void SaveConfigurations();

	void Read(const char* filePath);
};