#include "gif_frames.hpp"

GifFrames::GifFrames(ProgramConfiguration* program, CameraConfiguration* cameraConfig) 
	: program(program), camera(cameraConfig)
{	
	this->state = State::Initial;

	this->framesBefore = std::max(this->program->numberGifFrames.framesBefore, this->program->framesToAnalyzeChangeValidity.framesBefore);
	this->framesAfter = std::max(this->program->numberGifFrames.framesAfter, this->program->framesToAnalyzeChangeValidity.framesAfter);
	
	const size_t nframes = this->framesBefore + this->framesAfter;

	// framesTransformed.resize(nframes);
}

void GifFrames::addFrame(cv::Mat& frame) {
	if (this->updateBefore || this->before.size() < this->framesBefore) {
		if (this->before.size() > this->framesBefore)
			this->before.pop();

		this->before.push(frame.clone());
	} else if (this->updateAfter) { // change detected... get the following frames					 
		if (this->after.size() < this->framesAfter) {
			this->after.push(frame.clone());
		} else {
			// once we have all the frames...
			this->state = State::Ready;
			this->updateAfter = false;
			
			// move frames from queues to a vector
			this->framesToSingleVectors();
		}
	}
}

void GifFrames::framesToSingleVectors() {
	size_t currFrame = 0;

	while (!this->before.empty()) {
		this->frames.push_back(std::move(this->before.front()));
		
		if (currFrame < this->program->framesToAnalyzeChangeValidity.framesBefore) {
			FrameDescriptor fd;
			fd.frame = this->frames.back().clone();
			if (camera->rotation != 0)
				ImageManipulation::RotateImage(fd.frame, camera->rotation);

			fd.frame = fd.frame(camera->roi);

			cv::cvtColor(fd.frame, fd.frame, cv::COLOR_BGR2GRAY);

			this->framesTransformed.push_back(std::move(fd));
		}

		this->before.pop();
		currFrame++;
	}
	
	currFrame = 0;
	while (!this->after.empty()) {
		this->frames.push_back(std::move(this->after.front()));

		if (currFrame < this->program->framesToAnalyzeChangeValidity.framesAfter) {
			FrameDescriptor fd;
			fd.frame = this->frames.back().clone();

			if (camera->rotation != 0)
				ImageManipulation::RotateImage(fd.frame, camera->rotation);

			fd.frame = fd.frame(camera->roi);

			cv::cvtColor(fd.frame, fd.frame, cv::COLOR_BGR2GRAY);

			this->framesTransformed.push_back(std::move(fd));
		}

		this->after.pop();
		currFrame++;
	}
}

/// TODO: RETURN REF OR PNT
std::vector<cv::Mat> GifFrames::getGifFrames(bool applyTransformations) {
	return frames;
}

State GifFrames::getState() {
	return this->state;
}

void GifFrames::detectedChange() {
	this->updateBefore = false;
	this->updateAfter = true;
	this->state = State::Collecting;
}

bool GifFrames::isValid() {
	this->state = State::Wait;
	
	const size_t ammountOfFrames = this->program->framesToAnalyzeChangeValidity.framesBefore + this->program->framesToAnalyzeChangeValidity.framesAfter;	
	const double minPercentageAreaIgnore = camera->minPercentageAreaNeededToIgnore / 100;

	cv::Mat* frameCero;
	cv::Mat diff;
	
	size_t validFrames = 0;
	double totalDistance = 0;
	double totalAreaDifference = 0;
	double totalArea = 0;
	double totalNonPixels = 0;
	size_t overlappingFindings = 0;

	cv::Point p1;
	cv::Point p2;

	FindingInfo* lastValidFind = nullptr;

	//// Process frames

	bool p1Saved = false;
	for (size_t i = 1; i < ammountOfFrames; i++) {
		cv::absdiff(framesTransformed[i-1].frame, framesTransformed[i].frame, diff);
		
		cv::GaussianBlur(diff, diff, cv::Size(3, 3), 10);

		cv::threshold(diff, diff, camera->noiseThreshold, 255, cv::THRESH_BINARY);

		FindingInfo finding = FindRect(diff);
		framesTransformed[i].finding = finding;

		totalNonPixels += cv::countNonZero(diff);

		if (finding.isGoodMatch) {
			if (!p1Saved) {
				p1 = finding.center;
				p1Saved = true;
			}
			
			totalArea += finding.area;

//			cv::Point2f vertices[4];
//			finding.rect.points(vertices);
//			for (int j = 0; j < 4; j++) {
//				vertices[j].x += camera->roi.x;
//			}
			
			// check if finding is overlapping with a ignored area
			for (auto &&j : camera->ignoredAreas) {					
				cv::Rect inters = finding.rect.boundingRect() & j;
				if (inters.area() >= finding.rect.boundingRect().area() * minPercentageAreaIgnore) {
					overlappingFindings += 1;
					
					inters.x += camera->roi.x;
					inters.y += camera->roi.y;
					if (program->drawChangeFoundBetweenFrames)
						cv::rectangle(frames[i], inters, cv::Scalar(255, 0, 0), 1);
				}
			}
			
			// draw change (bounding rectangle)
			if (program->drawChangeFoundBetweenFrames) {
				cv::Rect bnd = finding.rect.boundingRect();
				bnd.x += camera->roi.x;
				bnd.y += camera->roi.y;
				cv::rectangle(frames[i], bnd, cv::Scalar(255,255,170), 1);
			}
			
			// draw change (rotated/original)
			// for (int j = 0; j < 4; j++) {			
			// 	cv::line(*frames[i], vertices[j], vertices[(j+1)%4], cv::Scalar(255,255,170), 1);
			// }

			if (lastValidFind != nullptr) {
				validFrames++;
				totalDistance += euclideanDist(framesTransformed[i].finding.center, lastValidFind->center);
				totalAreaDifference += abs(framesTransformed[i].finding.area - lastValidFind->area);
			}
		
			lastValidFind = &framesTransformed[i].finding;

			p2 = finding.center;
		}
	}

	double displacementX = abs(p1.x - p2.x);
	double displacementY = abs(p1.y - p2.y);

	this->debugMessage += "\ntotalNonPixels: " + std::to_string(totalNonPixels) + " totalAreaDifference: " + std::to_string(totalAreaDifference) + " total area % of non zero: " + std::to_string(totalAreaDifference * 100 / totalNonPixels);
	this->debugMessage += "\nP1: [" + std::to_string(p1.x) + "," + std::to_string(p1.y) + "] P2: [" + std::to_string(p2.x) + "," + std::to_string(p2.y) + "] Distance: " + std::to_string(euclideanDist(p1, p2)) + "\n DisplX: " + std::to_string(displacementX) + " DisplY: " + std::to_string(displacementY);
	this->debugMessage += "\nAverage area: " + std::to_string(totalArea / validFrames);
	if (validFrames != 0) {
		this->avrgDistanceFrames = totalDistance / validFrames;
		this->avrgAreaDifference = totalAreaDifference / validFrames;
	}

	bool valid = false;

	//// Check if it's valid
	if (validFrames != 0 && this->avrgDistanceFrames <= 120 && overlappingFindings < camera->thresholdFindingsOnIgnoredArea) {
		valid = true;

		this->state = State::Send;
	} else {
		this->state = State::Cancelled;
		// this->state = State::Send; // uncomment to send it any way

		// This if is only for debuggin purposes
		if (validFrames == 0) {
			this->avrgDistanceFrames = 0;
			this->avrgAreaDifference = 0;
			this->debugMessage += "\nCancelled due 0 valid frames found.";
		} else if (this->avrgDistanceFrames > 120) {
			this->debugMessage += "\nCancelled due avrg distance: " + std::to_string(this->avrgDistanceFrames);
		} else {
			this->debugMessage += "\nCancelled due to overlapping with ignored areas: " +  std::to_string(overlappingFindings) + " of " + std::to_string(camera->thresholdFindingsOnIgnoredArea) + " validFrames needed.";
		}
	}

	this->debugMessage += "\nAverage distance between 2 frames finding: " + std::to_string(this->avrgDistanceFrames) + "\naverage area difference: " + std::to_string(this->avrgAreaDifference) + "\n Valid frames: " + std::to_string(validFrames) + "\n overlapeds: " + std::to_string(overlappingFindings);
	
	std::cout << this->debugMessage << std::endl;

	return valid;
}

std::string GifFrames::getText() {
	return this->debugMessage;
}


size_t GifFrames::indexMiddleFrame() {
	return this->framesAfter == 0 ? this->framesBefore : this->framesBefore + 1;
}