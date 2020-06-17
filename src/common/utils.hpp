#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <iterator>

#include <time.h>
#include <ctime>
#include <stdio.h>



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
	static const std::string GetTimeFormated() {
		#ifdef WINDOWS
		time_t rawtime;
		struct tm timeinfo;
		char buffer[80];

		time(&rawtime);
		localtime_s(&timeinfo, &rawtime);

		strftime(buffer, 80, "%d_%m_%Y_%H_%M_%S", &timeinfo);
		return buffer;
		#else
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%d_%m_%Y_%H_%M_%S", &tstruct);

		return buf;
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

	static bool RectanglesOverlap(cv::Rect rect1, cv::Rect rect2){
		cv::Point r1(rect1.x + rect1.width, rect1.y + rect1.height);
		cv::Point r2(rect2.x + rect2.width, rect2.y + rect2.height);

		// is left to the other
		if (rect1.x >= r2.x || rect2.x >= r1.x) 
			return false; 
	
		// is above to the other
		if (rect1.y <= r2.y || rect2.y <= r1.y) 
			return false; 
	
		return true; 
	} 

	static float OverlappingArea(cv::Rect rect1, cv::Rect rect2) { 
		cv::Point r1(rect1.x + rect1.width, rect1.y + rect1.height);
		cv::Point r2(rect2.x + rect2.width, rect2.y + rect2.height);

		return (std::min(r1.x, r2.x) - std::max(rect1.x, rect2.x)) * 
				(std::min(r1.y, r2.y) - std::max(rect1.y, rect2.y));
	} 

	static AreasDelimiters StringToAreaDelimiters(char* str){		
		const char* delimiters = " [],()";
		AreasDelimiters adel;
		
		#ifdef WINDOWS
		char* next_token;
		char* res = strtok_s(&str[0], delimiters, &next_token);
		#else
		char* res = strtok(&str[0], delimiters);
		#endif
		
		uint8_t control = 0;
		while (res != NULL) {
			int fnd = std::stoi(res);
			if (control == 0)
				adel.rectEntry.x = fnd;
			else if (control == 1)
				adel.rectEntry.y = fnd;
			else if (control == 2)
				adel.rectEntry.width = fnd;
			else if (control == 3)
				adel.rectEntry.height = fnd;
			else if (control == 4)
				adel.rectExit.x = fnd;
			else if (control == 5)
				adel.rectExit.y = fnd;
			else if (control == 6)
				adel.rectExit.width = fnd;
			else if (control == 7)
				adel.rectExit.height = fnd;

			#ifdef WINDOWS
			res = strtok_s(NULL, delimiters, &next_token);
			#else
			res = strtok(NULL, delimiters);
			#endif
			
			control++;
		}

		return adel;
	}
	
	static std::string AreasDelimitersToString(const AreasDelimiters& adel) {
		std::ostringstream ss;
		ss << "[(" << adel.rectEntry.x << "," << adel.rectEntry.y
			<< "),(" << adel.rectEntry.width << "," << adel.rectEntry.height << ")]" 
			<< "[(" << adel.rectExit.x << "," << adel.rectExit.y
			<< "),(" << adel.rectExit.width << "," << adel.rectExit.height << ")]" ;
		return std::move(ss).str();
	}
};

