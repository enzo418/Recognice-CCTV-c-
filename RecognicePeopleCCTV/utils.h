#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <iterator>
#include <time.h>

static class Utils {
	public:
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
		static bool nextLineIsHeader(std::ifstream& file) {
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
			struct stat buffer;
			return (stat(name.c_str(), &buffer) == 0);
		}
		#pragma endregion

		#pragma region Sensibility_Time
		// <summary> Converts a string of e.g. "10 to 22 use 95" to Time class </summary>
		static Time GetTime(std::string str) {
			std::stringstream ss(str);

			int found, from, to, sensibility;
			int control = 0;
			std::string temp;
			Time time;

			while (std::getline(ss, temp, ' ')) {
				if (temp[0] >= '0' && temp[0] <= '9') {
					std::stringstream(temp) >> found;
					if (control == 0)
						from = found;
					else if (control == 1)
						to = found;
					else if (control == 2)
						sensibility = found;
					control++;
				}
			}

			if (control >= 2) {
				time.from = from;
				time.to = to;
				time.sensibility = sensibility;
			}

			return time;
		}

		static void DecodeSensibility(std::vector<Time>& time_list, std::string time_str) {
			if (time_str.size() == 0)
				return;

			char* next_token;
			char* res = strtok_s(&time_str[0], ",", &next_token);
			while (res != NULL) {
				time_list.push_back(GetTime(res));
				res = strtok_s(NULL, ",", &next_token);
			}
		}

		static inline void ShouldPickThisSens() {

		}
		#pragma endregion

		#pragma region RegionOfInteres
		static ROI GetROI(std::string roi_str) {
			ROI roi;

			const char* delimiters = " [],()";
			char* next_token;
			char* res = strtok_s(&roi_str[0], delimiters, &next_token);
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
				res = strtok_s(NULL, delimiters, &next_token);
				control++;
			}

			return roi;
		}
		#pragma endregion

		// <summary> Gets the current time and formats it to a format friendly for windows file name format </summary>
		static const char* GetTimeFormated() {
			time_t rawtime;
			struct tm timeinfo;
			char buffer[80];

			time(&rawtime);
			localtime_s(&timeinfo, &rawtime);

			strftime(buffer, 80, "%d_%m_%Y_%H_%M_%S", &timeinfo);
			return buffer;
		};

		// <summary> Gets the current hour</summary>
		static int GetCurrentHour(int& minutesLefToNextHour) {
			time_t theTime = time(NULL);
			struct tm aTime;
			localtime_s(&aTime, &theTime);

			minutesLefToNextHour = 60 - aTime.tm_min;
						
			return aTime.tm_hour;
		};
};

