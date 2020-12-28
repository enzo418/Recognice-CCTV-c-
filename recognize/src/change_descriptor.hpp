#pragma once

#include <opencv2/opencv.hpp>

#include <iostream>
#include <vector>

struct FindingInfo {
	double slope;
	double area;
	double angle;	
	double length;
	double height;
	cv::RotatedRect rect;
	cv::Point center;

	bool isGoodMatch;
};

typedef std::vector<std::vector<cv::Point>> ContourArray;

std::ostream& operator<<(std::ostream& out, FindingInfo const& n);
FindingInfo FindRect(cv::Mat& diffImage);
ContourArray FindRect(cv::Mat& diffImage, FindingInfo& finding);
double DistanceBetweenPoints(cv::Point p1, cv::Point p2);
void RemoveSmallContours(ContourArray& contours);
void CalculateReactangleContours(ContourArray& contours, cv::Size&& frameSz, FindingInfo& finding);
double CalculateSimilarityFindings(FindingInfo& f1, FindingInfo& f2);
double Map(double x, double in_min, double in_max, double out_min, double out_max);
cv::Mat DrawFinding(cv::Size&& imageSz, FindingInfo& finding, ContourArray& contours, bool drawContours);
void DrawFinding(cv::Mat& drawing, FindingInfo& finding, ContourArray& contours, bool drawContours);
bool IsAGoodMatch(ContourArray& contours, cv::Size&& frameSz);
double HeightContour(std::vector<cv::Point>& contour, cv::Point& left, cv::Point& right);
double euclideanDist(cv::Point2f& a, cv::Point2f& b);
double euclideanDist(cv::Point& a, cv::Point& b);