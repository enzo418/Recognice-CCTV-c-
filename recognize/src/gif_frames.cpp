#include "gif_frames.hpp"

GifFrames::GifFrames(ProgramConfiguration* program, CameraConfiguration* cameraConfig) 
	: program(program), camera(cameraConfig)
{	
	this->state = State::Initial;

	this->framesBefore = this->program->numberGifFrames.framesBefore;
	this->framesAfter = this->program->numberGifFrames.framesBefore;
	
	const size_t nframes = this->framesBefore + this->framesAfter;

	this->before.resize(nframes);
	this->after.resize(nframes);
	framesTransformed.resize(nframes);
}

void GifFrames::addFrame(cv::Mat& frame) {
	if (this->updateBefore || this->totalFramesBefore < this->framesBefore) {
		// increase total (if it's max then just leave it there)
		this->totalFramesBefore += this->totalFramesBefore >= this->framesBefore ? 0 : 1;

		// copy frame
		frame.copyTo(this->before[this->indexBefore]);

		// update next frame index
		size_t i = this->indexBefore;
		this->indexBefore = (i + 1) >= this->framesBefore ? 0 : (i + 1);
	} else if (this->updateAfter) { // change detected... get the following frames					 
		if (this->indexAfter < this->framesAfter) {
			frame.copyTo(this->after[this->indexAfter]);

			this->indexAfter++;
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
	const size_t ammountOfFrames = this->framesBefore + this->framesAfter;

	size_t totalFrames = 0;
	for (size_t i = this->indexBefore;;) {	
		if (totalFrames < this->framesBefore) {
			frames[totalFrames] = std::move(this->before[i]);
			
			framesTransformed[totalFrames].frame = frames[totalFrames].clone();

			if (camera->rotation != 0) ImageManipulation::RotateImage(framesTransformed[totalFrames].frame, camera->rotation);

			// Take the region of interes
			if (!camera->roi.empty()) {
				framesTransformed[totalFrames].frame = framesTransformed[totalFrames].frame(camera->roi);
			}

			// if (totalFrames == 0) {
			// 	frameCero = &framesTransformed[0].frame;
			// }

			totalFrames++;
			i = (i + 1) >= this->framesBefore ? 0 : (i + 1);
		} else
			break;
	}
	
	for (; totalFrames < ammountOfFrames; totalFrames++) {
		frames[totalFrames] = std::move(this->after[totalFrames - this->framesBefore]);
		
		framesTransformed[totalFrames].frame = frames[totalFrames].clone(); 

		if (camera->rotation != 0) ImageManipulation::RotateImage(framesTransformed[totalFrames].frame, camera->rotation);

		if (!camera->roi.empty()) {			
			framesTransformed[totalFrames].frame = framesTransformed[totalFrames].frame(camera->roi); 
		}
	}
}

/// TODO: RETURN REF OR PNT
std::vector<cv::Mat> GifFrames::getFrames(bool applyTransformations) {
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
	
	const size_t ammountOfFrames = this->framesBefore + this->framesAfter;	
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
	for (size_t i = 1; i < frames.size(); i++) {
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
					cv::rectangle(frames[i], inters, cv::Scalar(255, 0, 0), 1);
				}
			}
			
			cv::Rect bnd = finding.rect.boundingRect();
			bnd.x += camera->roi.x;
			bnd.y += camera->roi.y;
			cv::rectangle(frames[i], bnd, cv::Scalar(255,255,170), 1);

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