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
	// number of frames to wait until start saving the frames to analyze,
	// it's done like this to save the last <framesBefore> frames instead
	// of the first <framesBefore> of the before queue.
	int waitFrames = this->before.size() - this->program->framesToAnalyzeChangeValidity.framesBefore;

	while (!this->before.empty()) {
		this->frames.push_back(std::move(this->before.front()));

		if (waitFrames <= 0) {
			FrameDescriptor fd;
			fd.frame = this->frames.back().clone();
			if (camera->rotation != 0)
				ImageManipulation::RotateImage(fd.frame, camera->rotation);

			fd.frame = fd.frame(camera->roi);

			cv::cvtColor(fd.frame, fd.frame, cv::COLOR_BGR2GRAY);

			this->framesTransformed.push_back(std::move(fd));			
		}

		this->before.pop();
		waitFrames--;
	}

	size_t currFrame = 0;
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

	const int offset = this->program->numberGifFrames.framesBefore - this->program->framesToAnalyzeChangeValidity.framesBefore;

	const cv::Point imageCenter = cv::Point(RESIZERESOLUTION.width / 2, RESIZERESOLUTION.height / 2);
	// const cv::Point imageCenter = cv::Point(camera->roi.width / 2, camera->roi.height / 2);

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
			
			if (offset + i >= 0) {
				const cv::Point rotatedCenter = Utils::RotatePointAround(finding.center, -1 * camera->rotation, cv::Point(0,0));
				const cv::Rect rotatedFinding(
					cv::Point(
						rotatedCenter.x - finding.rect.size.width / 2,
						rotatedCenter.y - finding.rect.size.height / 2
					), finding.rect.size);
				std::cout 	<< "[GIF] Rotated center: " << rotatedCenter
							<< "\n      From: " << finding.center
							<< "\n      With angle: " << camera->rotation
							<< "\n      Around: " << imageCenter
							<< std::endl;

				// const cv::Point centerRotated = cv::Point(
				// 		rotatedFinding.x + rotatedFinding.width / 2,
				// 		rotatedFinding.y + rotatedFinding.height / 2
				// 	);

				this->findings.push_back(
					std::make_tuple(
						offset + i, // finding index on "frames" member
						rotatedFinding, 
						cv::Point(rotatedCenter.x + this->camera->roi.x, rotatedCenter.y + this->camera->roi.y),
						cv::Point(finding.center.x + this->camera->roi.x, finding.center.y + this->camera->roi.y)
					)
				);
			}

			// check if finding is overlapping with a ignored area
			for (auto &&j : camera->ignoredAreas) {					
				cv::Rect inters = finding.rect.boundingRect() & j;
				if (inters.area() >= finding.rect.boundingRect().area() * minPercentageAreaIgnore) {
					overlappingFindings += 1;
				}
			}

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
		
		// ---------------------------------------
		//  Draw trace and findings on the frames
		// ---------------------------------------

		const bool drawTrace = this->program->drawTraceOfChangeFoundOn == DrawTraceOn::Both 
								|| this->program->drawTraceOfChangeFoundOn == DrawTraceOn::Gif;
		
		size_t currFinding_i = 0; 
		std::tuple<size_t, cv::Rect, cv::Point, cv::Point> currFinding = this->findings[currFinding_i];
		bool savedFirstFrameWithFinding = false;

		std::vector<cv::Point> pointsDrawn;

		for (size_t i = 0; i < frames.size(); i++) {
			cv::Mat& frame = frames[i];
			
			// if current finding "frames" index is i
			if (std::get<0>(currFinding) == i) {
				// save the first frame in which a change has been found
				if (!savedFirstFrameWithFinding) {
					frame.copyTo(this->firstFrameWithDescription);
					savedFirstFrameWithFinding = true;
				}

				/// -- Test
				cv::circle(frame, std::get<3>(currFinding), 3, cv::Scalar(255, 50, 50), -1);
				/// -- Test

				// draw finding rectangle
				if (program->drawChangeFoundBetweenFrames) {
					cv::Rect bnd = std::get<1>(currFinding);
					bnd.x += camera->roi.x;
					bnd.y += camera->roi.y;
					cv::rectangle(frame, bnd, cv::Scalar(255,255,170), 1);
				}

				pointsDrawn.push_back(std::get<2>(currFinding));

				// go to next finding
				if (currFinding_i + 1 < this->findings.size()) {
					currFinding_i += 1;
					currFinding = this->findings[currFinding_i];
				}
			}


			// draw all the trace points (finding center) and the lines between them
			if (this->program->drawTraceOfChangeFoundOn == DrawTraceOn::Both 
				|| this->program->drawTraceOfChangeFoundOn == DrawTraceOn::Gif) {
				for (size_t j = 0; j < pointsDrawn.size(); j++) {
					cv::circle(frame, pointsDrawn[j], 2, cv::Scalar(0, 0, 255), -1);
					
					if (j + 1 < pointsDrawn.size()) {
						cv::line(frame, pointsDrawn[j], pointsDrawn[j+1], cv::Scalar(0,255,0));
					}
				}
			}
		}
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

cv::Mat& GifFrames::firstFrameWithChangeDetected(){
	return this->firstFrameWithDescription;
}

std::vector<std::tuple<size_t, cv::Rect, cv::Point, cv::Point>> GifFrames::getFindingsTrace() {
	return this->findings;
}