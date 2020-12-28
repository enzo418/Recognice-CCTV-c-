#include "change_descriptor.hpp"

FindingInfo FindRect(cv::Mat& diffImage) {	
	FindingInfo finding;

	ContourArray contours;
	cv::RotatedRect contoursRect;

	cv::findContours(diffImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	
	RemoveSmallContours(contours);

	finding.isGoodMatch = IsAGoodMatch(contours, diffImage.size());

	if (finding.isGoodMatch) {
		CalculateReactangleContours(contours, diffImage.size(), finding);		
	}

	return finding;
}

ContourArray FindRect(cv::Mat& diffImage, FindingInfo& finding) {	
	ContourArray contours;
	cv::RotatedRect contoursRect;

	cv::findContours(diffImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
	
	RemoveSmallContours(contours);

	finding.isGoodMatch = IsAGoodMatch(contours, diffImage.size());

	if (finding.isGoodMatch) {
		CalculateReactangleContours(contours, diffImage.size(), finding);		
	}

	return contours;
}

cv::Mat DrawFinding(cv::Size&& imageSz, FindingInfo& finding, ContourArray& contours, bool drawContours = true) {
	cv::RNG rng(12345);

	// Draw contours and extreme points
	cv::Mat drawing = cv::Mat::zeros(imageSz, CV_8UC3);
	cv::Scalar color;
	if (drawContours) {
		for(size_t i = 0; i < contours.size(); i++) {
			color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
			cv::drawContours(drawing, contours, (int)i, color, 2, cv::LINE_8);
		}
	}

	color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
	cv::Point2f vertices[4];
	finding.rect.points(vertices);
	for (int i = 0; i < 4; i++)
		cv::line(drawing, vertices[i], vertices[(i+1)%4], color, 2);	

	return drawing;
}

void DrawFinding(cv::Mat& drawing, FindingInfo& finding, ContourArray& contours, bool drawContours = true) {
	cv::RNG rng(12345);

	// Draw contours and extreme points	
	cv::Scalar color;
	if (drawContours) {
		for(size_t i = 0; i < contours.size(); i++) {
			color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
			cv::drawContours(drawing, contours, (int)i, color, 2, cv::LINE_8);
		}
	}

	color = cv::Scalar(rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
	cv::Point2f vertices[4];
	finding.rect.points(vertices);
	for (int i = 0; i < 4; i++)
		cv::line(drawing, vertices[i], vertices[(i+1)%4], color, 2);
}

void RemoveSmallContours(ContourArray& contours) {
	double avrgSlope = 0;
	
	std::vector<double> slopes;

	size_t size = contours.size();
	for (size_t i = 0; i < size; i++) {
		cv::Point l = contours[i][0];
		cv::Point r = contours[i][0];
		cv::Point t = contours[i][0];
		cv::Point b = contours[i][0];

		for (size_t j = 1; j < contours[i].size(); j++) {
			if (contours[i][j].x < l.x) l = contours[i][j];
			if (contours[i][j].x > r.x) r = contours[i][j];
			if (contours[i][j].y > t.y) t = contours[i][j];
			if (contours[i][j].y < b.y) b = contours[i][j];
		}

		double height = HeightContour(contours[i], l, r);

		double area = euclideanDist(l, r) * euclideanDist(t, b);

		if (area < 7) {
			contours.erase(contours.begin() + i);
			i--;
			size--;
		} else {
			double slope = ((double)(l.y - r.y) / (double)(l.x - r.x));
			slopes.push_back(slope);	
			if (i != 0)		 
				avrgSlope = (avrgSlope + slope) / 2;
			else
				avrgSlope = slope;
		}
	}

	size = slopes.size();
	for (size_t i = 0; i < size; i++) {
		if(!(slopes[i] > avrgSlope - 0.5 && slopes[1] < avrgSlope + 0.5)) {
			slopes.erase(slopes.begin() + i);
			contours.erase(contours.begin() + i);
			i--;
			size--;
		}
	}	
}

void CalculateReactangleContours(ContourArray& contours, cv::Size&& frameSz, FindingInfo& finding) {
	double averageHeight = 0;

	cv::Point l = contours[0][0];
	cv::Point r (0,0);

	double maxHeight = 0;
	double minHeight = frameSz.height;

	// int last = 0;
	for (size_t i = 0; i < contours.size(); i++) {
		cv::Point localLeft = contours[i][0];
		cv::Point localRight = contours[i][0];

		// Find local left and local right (extremes)
		for (size_t j = 0; j < contours[i].size(); j++) {			
			if (contours[i][j].x < localLeft.x) localLeft = contours[i][j];
			if (contours[i][j].x > localRight.x) localRight = contours[i][j];
		}

		// Find the height of this contour
		double height = HeightContour(contours[i], localLeft, localRight);
		
		if (localLeft.x < l.x) l = localLeft;
		if (localRight.x > r.x) r = localRight;

		// double height = abs(middlePoint.y - farYMiddlePoint.y);
		
		if (i != 0) { 
			averageHeight = (height + averageHeight) / 2;
		} else {
			averageHeight += height;
		}

		if (height > maxHeight) { 
			maxHeight = height;
		}
		
		if (height < minHeight) {
			minHeight = height;
		}

		// save the points to show it
		// tb[i+last] = middlePoint;
		// tb[i+1+last] = farYMiddlePoint;
		// last++;
	}	

	// averageHeight = (averageHeight + maxHeight + minHeight*0.8) / 3;
	averageHeight = maxHeight;

	double lineLength = euclideanDist(l, r);
	double centerX = ((r.x - lineLength / 2) + (l.x + lineLength / 2))/2;
	finding.slope = ((double)(l.y - r.y) / (double)(l.x - r.x));
	double centerY = ((finding.slope * (centerX - r.x) + r.y) + (finding.slope * (centerX - l.x) + l.y)) / 2;

	cv::Point2f center (centerX, centerY);

	finding.angle = (atan((double)(l.y - r.y) / (double)(l.x - r.x))) * 180 / M_PI;

	finding.rect = cv::RotatedRect(center, cv::Size(lineLength, averageHeight), finding.angle);

	finding.length = lineLength;
	finding.height = averageHeight;

	finding.area = lineLength * averageHeight;

	finding.center = center;
}

double CalculateSimilarityFindings(FindingInfo& f1, FindingInfo& f2) {
	double probability = 0;
	// TOTAL = 1

	// Angle is rated from 0 to 0.5
	probability += Map(abs(f1.angle - f2.angle), 70, 0, 0, 0.5);

	// Displacement of center is rated between 0 and 0.1 fro X and 0 to 0.4 for Y
	probability += Map(abs(f1.center.x - f2.center.x), 50, 0, 0, 0.1);
	probability += Map(abs(f1.center.y - f2.center.y), 50, 0, 0, 0.4);

	return probability;
}

double Map(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool IsAGoodMatch(ContourArray& contours, cv::Size&& frameSz) {
	bool isGoodMatch = false;
	if (contours.size() > 0) {
		// Isn't a good match if the contours are all over the screen, even when we deleted the small ones...
		cv::Point lt = contours[0][0];
		cv::Point rt = contours[0][0];
		cv::Point lb = contours[0][0];
		cv::Point rb = contours[0][0];
		
		for (size_t i = 0; i < contours.size(); i++) {
			for (size_t j = 0; j < contours[i].size(); j++) {
				cv::Point p = contours[i][j];
				if (p.x <= lt.x && p.y <= lt.y) lt = p;
				if (p.x <= lb.x && p.y >= lb.y) lb = p;

				if (p.x >= rt.x && p.y <= rt.y) rt = p;
				if (p.x >= rb.x && p.y >= rb.y) rb = p;
			}		
		}	

		double width = (euclideanDist(lt, rt) + euclideanDist(lb, rb)) / 2;
		double height = (euclideanDist(lt, lb) + euclideanDist(rt, rb)) / 2;
		isGoodMatch = frameSz.width * 0.7 > width && frameSz.height * 0.7 > height;
		isGoodMatch = isGoodMatch && width * height > 5;
	}

	return isGoodMatch;
}

double HeightContour(std::vector<cv::Point>& contour, cv::Point& left, cv::Point& right) {
	double middleX = left.x + euclideanDist(left, right) / 2;
	cv::Point middlePoint (-1,-1);
	cv::Point farYMiddlePoint (-1,-1);

	for (size_t j = 0; j < contour.size(); j++) {			
		// if the point is in E(middleX, 2)
		if (contour[j].x <= middleX + 2 && contour[j].x >= middleX - 2) {
			// if none was taken
			if (middlePoint.x == -1 && middlePoint.y == -1){
				middlePoint = contour[j];
				farYMiddlePoint = middlePoint;				
			} 
			// else if the distance between this point and the middle selectes is > last one taked
			else if (abs(middlePoint.y - contour[j].y) > abs(middlePoint.y - farYMiddlePoint.y)){
				farYMiddlePoint = contour[j];
			}
		}
	}

	return abs(middlePoint.y - farYMiddlePoint.y);
}

double euclideanDist(cv::Point2f& a, cv::Point2f& b) {
	cv::Point2f diff = a - b;
	return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

double euclideanDist(cv::Point& a, cv::Point& b) {
	cv::Point diff = a - b;
	return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}


std::ostream& operator<<(std::ostream& out, FindingInfo const& n) {
	out << "Good Match: " << (n.isGoodMatch ? "true" : "false")
		<< " Center: " << n.center 
		<< " Angle: " << n.angle 
		<< " Area: " << n.area
		<< " Slope: " << n.slope;
	return out;
}