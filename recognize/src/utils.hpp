#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include <iterator>
#include <algorithm>
#include <time.h>
#include <ctime>
#include <cmath>
#include <stdio.h>
#include <regex>
#include <opencv2/opencv.hpp>

#include "types.hpp"
#include "types_configuration.hpp"

#define ExistInVector(vector,val) (std::find(std::begin(vector), std::end(vector), val) != std::end(vector))

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
	// template<typename S>
	static bool nextLineIsHeader(std::istream& file) {
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

	static void inline LogTime(tm* time){
		std::cout << time->tm_mday << "/" << time->tm_mon << "/" << time->tm_year << " "
				<< time->tm_hour << ":" << time->tm_min << ":" << time->tm_sec << ":";
	}

	/// <summary> Fix the member "order" so it start from 0 and we don't skip any number between cameras </summary>
	static void FixOrderCameras(CamerasConfigurations& cameras) {	
		std::sort(cameras.begin(), cameras.end(), less_than_order);
		int size = cameras.size();
		for (int expected = 0; expected < size; expected++) {
			if (cameras[expected].order != expected) {
				cameras[expected].order = expected;
			}
		}			
	}

	static std::string PointToString(cv::Point rt){
		return "(" + std::to_string(rt.x)  + "," + std::to_string(rt.y) + ")";
	}

	static std::string RectangleToString(cv::Rect rt, bool leftUpRighBtm){
		if(!leftUpRighBtm)
			return "[(" + std::to_string(rt.x)  + "," + std::to_string(rt.y) + "),("+std::to_string(rt.width)+","+std::to_string(rt.height) + ")]";
		else
			return "[(" + std::to_string(rt.tl().x)  + "," + std::to_string(rt.tl().y) + "),("+std::to_string(rt.br().x)+","+std::to_string(rt.br().y) + ")]";
	}

	static cv::Point TopLeftRectangle(cv::Rect rect){
		return cv::Point(rect.x, rect.y+rect.height);
	}

	static cv::Point BottomRightRectangle(cv::Rect rect){
		return cv::Point(rect.x + rect.width, rect.y);
	}

	static bool RectanglesOverlap(cv::Rect rect1, cv::Rect rect2){
		// top left point
		cv::Point tl1 = TopLeftRectangle(rect1);
		cv::Point tl2 = TopLeftRectangle(rect2);

		// bottom righ point
		cv::Point br1 = BottomRightRectangle(rect1);
		cv::Point br2 = BottomRightRectangle(rect2);

		// is left to the other
		if (tl1.x >= br2.x || tl2.x >= br1.x) 
			return false; 
	
		// is above to the other
		if (tl1.y <= br2.y || tl2.y <= br1.y) 
			return false; 
	
		return true;
	} 

	static int OverlappingArea(cv::Rect rect1, cv::Rect rect2) { 
		// top left point
		cv::Point tl1 = TopLeftRectangle(rect1);
		cv::Point tl2 = TopLeftRectangle(rect2);

		// bottom righ point
		cv::Point br1 = BottomRightRectangle(rect1);
		cv::Point br2 = BottomRightRectangle(rect2);

		// std::cout << " Points =>  1. [" << PointToString(tl1) << "," << PointToString(br1) << "]"
		// 		<<   "\n          2. [" << PointToString(tl2) << "," << PointToString(br2) << "]"
		// << std::endl;

		if(RectanglesOverlap(rect1, rect2)){

			const int area = abs((std::max(tl1.x, tl2.x) - std::min(br1.x, br2.x)) * 
					(std::max(tl1.y, tl2.y) - std::min(br1.y, br2.y)));

			//std::cout << " Will return area = " << area << std::endl;

			return area;
		}else{
			// std::cout << " Rectangles doesn't overlap =>  1. [(" << rect1.x <<"," << rect1.y << "),("<<rect1.width<<","<<rect1.height << ")]"
			// << " 2. [(" << rect2.x <<"," << rect2.y << "),("<<rect2.width<<","<<rect2.height << ")]"
			// << std::endl;

			// std::cout << " Rectangles doesn't overlap => 1. " << RectangleToString(rect1, true) 
			// << "\n 			2. " << RectangleToString(rect2, true)
			// << std::endl;

			// std::cout << " Rectangles doesn't overlap => 1. [" << PointToString(tl1) << "," << PointToString(br1) << "]"
			// << "\n 				2. " << PointToString(tl2) << "," << PointToString(br2) << "]"
			// << std::endl;
			return 0;
		}
	}
	
	static std::vector<std::string> SplitString(const std::string& str, const std::string& delim) {
		std::vector<std::string> tokens;
		size_t prev = 0, pos = 0;
		do {
			pos = str.find(delim, prev);
			if (pos == std::string::npos) pos = str.length();
			std::string token = str.substr(prev, pos-prev);
			trim(token);
			tokens.push_back(token);
			prev = pos + delim.length();
		} while (pos < str.length() && prev < str.length());

		return tokens;
	}

	static std::string MatType(cv::Mat& inputMat) {
		int inttype = inputMat.type();

		std::string r, a;
		uchar depth = inttype & CV_MAT_DEPTH_MASK;
		uchar chans = 1 + (inttype >> CV_CN_SHIFT);
		switch ( depth ) {
			case CV_8U:  r = "8U";   a = "Mat.at<uchar>(y,x)"; break;  
			case CV_8S:  r = "8S";   a = "Mat.at<schar>(y,x)"; break;  
			case CV_16U: r = "16U";  a = "Mat.at<ushort>(y,x)"; break; 
			case CV_16S: r = "16S";  a = "Mat.at<short>(y,x)"; break; 
			case CV_32S: r = "32S";  a = "Mat.at<int>(y,x)"; break; 
			case CV_32F: r = "32F";  a = "Mat.at<float>(y,x)"; break; 
			case CV_64F: r = "64F";  a = "Mat.at<double>(y,x)"; break; 
			default:     r = "User"; a = "Mat.at<UKNOWN>(y,x)"; break; 
		}   
		r += "C";
		r += (chans+'0');
		return r  + " and should be accessed with " + a;
	}

	static std::vector<int> GetNumbersString(std::string s) {
		std::regex r("(-?[0-9]*)"); 
		std::vector<int> results;
		for(std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r); i != std::sregex_iterator(); ++i)  { 
			std::smatch m = *i; 
			if (!m[1].str().empty())
				results.push_back(std::stoi(m[1].str()));
		} 
		return results;
	}

	static std::vector<std::string> GetRange(std::string s) {
		std::regex r("(-?[0-9]*)?(..)(-?[0-9]*)?"); 
		std::vector<std::string> results;
		for(std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r); i != std::sregex_iterator(); ++i)  { 
			std::smatch m = *i; 
			for(int j = 0; j < m.size(); j++)
				if (!m[j].str().empty() && j != 0)
					results.push_back(m[j].str());
		} 
		
		return results;
	}

	/**
	 * @brief Generates a comma separeted string with the values of a vector
	 * @param v vector of strings
	 * @return a string
	 */
	static std::string VectorToCommaString(std::vector<std::string>&  v) {
		std::string s;
		
		for (size_t i = 0; i < v.size(); i++) {
			trim(v[i]);
			s += v[i];
			if (i != v.size() - 1)
				s += ",";
		}
		
		return s;
	}

	static std::string IgnoredAreasToString(std::vector<cv::Rect>& ia) {
		std::string s;
		
		for (size_t i = 0; i < ia.size(); i++) {
			std::ostringstream ss;
			ss << "[(" << ia[i].x << ", " << ia[i].y << "),(" << ia[i].width << ", " << ia[i].height << ")]";
			s += ss.str();
			if (i != ia.size() - 1)
				s += ",";
		}
		
		return s;
	}
		
	static std::string RectToCommaString(const cv::Rect & roi) {
		std::ostringstream ss;
		ss << roi.x << "," << roi.y << "," << roi.width << "," << roi.height;
		return ss.str();
	}

	static std::string FormatNotificationTextString(std::string str, const std::string& name) {
		// Implementation of a quick (to write) replace method
		size_t pos = str.find("{N}");
		
		if (pos != std::string::npos) {		
			return str.replace(pos, 3, name);
		}

		return name;
	}

	static cv::Point RotatePointAround(const cv::Point& point, int theta, const cv::Point& around) {
		const double rad = theta * M_PI / 180;
		const double cosThetha = cos(rad), sinThehta = sin(rad);
		const double x2 = cosThetha * (point.x - around.x) + sinThehta * (point.y - around.y);
		const double y2 = -sinThehta * (point.x - around.x) + cosThetha * (point.y - around.y);
		return cv::Point(x2, y2);
		
		/// TODO: this

		// return point;
	}

	static const std::vector<std::string> GetTokensDiscriminatorArea(const std::string& s, ushort& numberDiscriminator) {
		std::regex r("(-)?(?:(allow|deny):)?([0-9]+)");
        std::vector<std::string> results;
		numberDiscriminator = 1; // always is >= 1 since s is not empty
        for(std::sregex_iterator i = std::sregex_iterator(s.begin(), s.end(), r); i != std::sregex_iterator(); ++i)  { 
			std::smatch m = *i; 
			for(int j = 0; j < m.size(); j++) {
				const std::string& str = m[j].str();
				if (!str.empty() && j != 0) {
					numberDiscriminator += str == "-" ? 1 : 0;
					results.push_back(str);
				}
			}
        }

		return results;
	}

	static bool String2DiscriminatorArea(const std::string& s, std::vector<PointsDiscriminatorArea>& discriminators) {
		bool sucess = true;

		ushort numberDiscriminator;
		const std::vector<std::string> tokens = GetTokensDiscriminatorArea(s, numberDiscriminator);
		
		discriminators.resize(numberDiscriminator);
		ushort currDisc = 0;
		
		bool isNewPoint = true;
		for (size_t i = 0; i < tokens.size(); i++) {
			const std::string& tk = tokens[i];
			if (tk == "-") {
				currDisc++;
			} else if (tk == "allow" || tk == "deny") {
				discriminators[currDisc].type = (tk == "allow" ? DiscriminatorType::Allow : DiscriminatorType::Deny);
			} else {
				// parse the string to a integer, we are sure it is an intenger
				int n_x = std::stoi(tk);
				
				// we also know that this token is x and the next one is y
				int n_y = std::stoi(tokens[i+1]);

				// push the new point
				discriminators[currDisc].points.push_back(cv::Point(n_x, n_y));

				// skip the next token since we already use it as n_y
				i++;
			}
		}

		return sucess;	
	}

	static const std::string DiscriminatorArea2String(std::vector<PointsDiscriminatorArea>& discriminators) {
		std::string s;
		bool isFirst = true;
		for (auto &&d : discriminators) {			
			s += (isFirst ? "" : "-");
			s += (d.type == DiscriminatorType::Allow ? "allow" : "deny");
			s += ":";
			
			for (auto &&p : d.points) {
				s += std::to_string(p.x) + "," + std::to_string(p.y) + ",";
			}
			
			s.pop_back(); // remove last ,
			isFirst = false;
		}
		
		return s;
	}
};

