#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <iterator>
#include <time.h>

namespace Utils {
	#pragma region StringManipulation

	static void toLowerCase(char*& str) {
		int length = strlen(str);
		for (size_t i = 0; i < length; i++) {
			str[i] = tolower(str[i]);
		}
	}

	static void toLowerCase(std::string& str) {
		int length = str.size();
		for (size_t i = 0; i < length; i++) {
			str[i] = tolower(str[i]);
		}
	}

	static char* substr(char* arr, int begin, int len) {
		char* res = new char[len];
		for (int i = 0; i < len; i++)
			res[i] = *(arr + begin + i);
		res[len] = 0;
		return res;
	}

	static inline void trim(std::string& s) {
		s.erase(s.find_last_not_of(" \n\r\t") + 1);
	}
	#pragma endregion

	#pragma region ConfigFileHelpers
	static bool nextLineIsHeader(std::fstream& file) {
		char c = 0;
		char last = 0;

		if (file.good()) {
			file.get(c);
			
			if (c == '\n' && file.good()) {
				file.get(c);

				// go back 2 positions
				file.unget();
				file.unget();

				return c == ']';
			}

			// go back 1 position
			file.unget();
			return c == '[';
		}

		return false;
	}
	#pragma endregion

	#pragma region File
	static inline bool FileExist(const std::string& name) {
		std::ifstream f(name.c_str());
		return f.good();
		
		//struct stat buffer;
		//return (stat(name.c_str(), &buffer) == 0);
	}
	#pragma endregion

	#pragma region RegionOfInteres
	static ROI GetROI(std::string roi_str) {
		ROI roi;

		const char* delimiters = " [],()";
		
		#ifdef WINDOWS
		char* next_token;
		char* res = strtok_s(&roi_str[0], delimiters, &next_token);
		#else
		char* res = strtok(&roi_str[0], delimiters);
		#endif
		
		uint8_t control = 0;
		while (res != NULL) {
			int fnd = std::stoi(res);
			if (control == 0)
				roi.point1.x = fnd;
			else if (control == 1)
				roi.point1.y = fnd;
			else if (control == 2)
				roi.point2.x = fnd;
			else if (control == 3)
				roi.point2.y = fnd;
				
			#ifdef WINDOWS
			res = strtok_s(NULL, delimiters, &next_token);
			#else
			res = strtok(NULL, delimiters);
			#endif
			
			control++;
		}

		return roi;
	}

	static std::string RoiToString(const ROI& roi) {
		std::ostringstream ss;
		ss << "[(" << roi.point1.x << "," << roi.point1.y
			<< "),(" << roi.point2.x << "," << roi.point2.y << ")]";
		return std::move(ss).str();
	}
	#pragma endregion

	static std::string CamTypeToString(CAMERATYPE const& type) {
		if (type == CAMERA_DISABLED) return "Disabled";
		else if (type == CAMERA_SENTRY) return "Sentry";
		else return "Active";
	}

	/// <summary> Gets the current time and formats it to a format friendly for windows file name format </summary>
	static const char* GetTimeFormated() {
		#ifdef WINDOWS
		time_t rawtime;
		struct tm timeinfo;
		char buffer[80];

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);

		strftime(buffer, 80, "%d_%m_%Y_%H_%M_%S", &timeinfo);
		return buffer;
		#else
		std::time_t t = std::time(nullptr);
		std::tm tm = *std::localtime(&t);
		
		std::stringstream buffer;
		buffer << std::put_time(&tm, "%d_%m_%Y_%H_%M_%S");
		
		return buffer.str().c_str();
		#endif
	};

	/// <summary> Gets the current hour</summary>
	static int GetCurrentHour(int& minutesLefToNextHour) {
		time_t theTime = time(NULL);
		#ifdef WINDOWS
		struct tm aTime;
		localtime_s(&aTime, &theTime);
		minutesLefToNextHour = 60 - aTime.tm_min;						
		return aTime.tm_hour;
		#else
		struct tm *aTime = localtime(&theTime);
		minutesLefToNextHour= 60 - aTime->tm_min;
		return aTime->tm_hour;
		#endif
	};

	/// <summary> Fix the member "order" so it start from 0 and we don't skip any number between cameras </summary>
	static void FixOrderCameras(std::vector<CameraConfig>& cameras) {	
		std::sort(cameras.begin(), cameras.end(), less_than_order);
		int size = cameras.size();
		for (int expected = 0; expected < size; expected++) {
			if (cameras[expected].order != expected) {
				cameras[expected].order = expected;
			}
		}			
	}
};

