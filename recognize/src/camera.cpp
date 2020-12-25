#include "camera.hpp"

Camera::Camera(CameraConfiguration& cameraConfig, ProgramConfiguration* programConfig, bool* stopFlag, cv::HOGDescriptor* hog) : config(&cameraConfig), _programConfig(programConfig), _stop_flag(stopFlag), _descriptor(hog) {
	this->gifFrames.after.resize(programConfig->numberGifFrames.framesAfter);
	this->gifFrames.before.resize(programConfig->numberGifFrames.framesBefore);
	this->Connect();
	this->accumulatorThresholds = cameraConfig.minimumThreshold;
}

void Camera::Connect() {
	this->capturer.open(this->config->url);
}

std::thread Camera::StartDetection() {
	return std::thread(&Camera::ReadFramesWithInterval, this);
}

// Cuts the image, applies the desired rotation and the converts the image to white and black.
void Camera::ApplyBasicsTransformations() {
	// if use gif is true, it's already resized
	if (!this->_programConfig->useGifInsteadImage)
		cv::resize(this->frame, this->frame, RESIZERESOLUTION);
	
	this->frameToShow = this->frame.clone();

	// auto intervalFrames = (now - this->lastBackupImageStored) / std::chrono::seconds(1);
	// if (intervalFrames >= 10) {
	// 	this->imageFrom10SecondsAgo = this->imageFrom10SecondsAgo;
	// 	this->lastBackupImageStored = std::chrono::high_resolution_clock::now();
	// }

	// Then rotate it
	if (this->config->rotation != 0) ImageManipulation::RotateImage(this->frame, this->config->rotation);

	// Take the region of interes
	if (!this->config->roi.empty()) {
		this->frame = this->frame(this->config->roi);
	}

	cv::cvtColor(this->frame, this->frame, cv::COLOR_RGB2GRAY);

	// Don't use hight constrast here... it only increases the change between images.
	// is more usefull when trying to detect objs.
}

void Camera::CalculateNonZeroPixels() {
	cv::absdiff(this->lastFrame, frame, diff);

	cv::GaussianBlur(diff, diff, cv::Size(3, 3), 10);

	// 45 works perfect with most of the cameras/resolution
	cv::threshold(diff, diff, this->config->noiseThreshold, 255, cv::THRESH_BINARY);

	totalNonZeroPixels = cv::countNonZero(diff);

	if (this->_programConfig->showProcessedFrames && this->_programConfig->showPreview) {
		cv::Mat d2;
		// place diff image on top the frame img
		cv::addWeighted(frame, 1, diff, 8, 12, d2);
		
		this->frames.push_back(d2);
	}
}

void Camera::UpdateThreshold() {
	this->accumulatorThresholds += this->config->minimumThreshold + this->totalNonZeroPixels;
	this->thresholdSamples++;
		
	if(this->thresholdSamples == 0 /*|| this->accumulatorThresholds == 0*/) return;

	auto time = (this->now - this->lastThresholdUpdate) / std::chrono::seconds(1);
	if(time >= this->config->updateThresholdFrequency) {
		const int oldThres = this->config->changeThreshold;

		this->config->changeThreshold = this->accumulatorThresholds / this->thresholdSamples * this->config->increaseTresholdFactor;

		std::cout << "[I] Threshold " << this->config->cameraName 
				<< " freq=" << this->config->updateThresholdFrequency
				<< " chaged from: " << oldThres << " to: " << this->config->changeThreshold 
				<< " [accumulator=" << accumulatorThresholds
				<< " / samples=" << this->thresholdSamples
				<< "] => average=" << this->accumulatorThresholds / this->thresholdSamples
				<< " (factor = " << this->config->increaseTresholdFactor << ")"
				<< std::endl;
			
		this->accumulatorThresholds = 0;
		this->thresholdSamples = 0;

		this->lastThresholdUpdate = std::chrono::high_resolution_clock::now();
	}
}

void Camera::ChangeTheStateAndAlert(std::chrono::system_clock::time_point& now) {
		// Play a sound
		this->pendingNotifications.push_back(Notification::Notification());
		
		// Send message with image
		auto intervalFrames = (now - this->lastImageSended) / std::chrono::seconds(1);
		if (intervalFrames >= this->_programConfig->secondsBetweenImage) {
			if(this->_programConfig->sendImageWhenDetectChange && !this->_programConfig->useGifInsteadImage){                  
				Notification::Notification imn(this->frameToShow, "Movimiento detectado en esta camara.", false);
				this->pendingNotifications.push_back(imn);
			} else if (this->_programConfig->sendImageWhenDetectChange && this->_programConfig->useGifInsteadImage) {			
				this->gifFrames.updateBefore = false;
				this->gifFrames.updateAfter = true;
				this->gifFrames.state = State::Collecting;
			}
			
			this->lastImageSended = std::chrono::high_resolution_clock::now();
		}
		
		// Send text message
		intervalFrames = (now - this->lastTextSended) / std::chrono::seconds(1);
		if (intervalFrames >= this->_programConfig->secondsBetweenMessage) {
			if (this->_programConfig->sendTextWhenDetectChange){
				Notification::Notification imn("Movimiento detectado en la camara " + this->config->cameraName);
				this->pendingNotifications.push_back(imn);
				this->lastTextSended = std::chrono::high_resolution_clock::now();
			}
		}

}

void Camera::ReadFramesWithInterval() {
	// ==============
	//  consts/vars
	// ==============
	
	ushort framesLeft = 0; 

	const ushort interval = this->_programConfig->msBetweenFrame;
	
	// higher interval -> lower max frames & lower interval -> higher max frames
	const ushort maxFramesLeft = (100.0 / (interval+0.0)) * 70; // 100 ms => max = 70 frames

	const char* camName = &this->config->cameraName[0];
	const CAMERATYPE camType = this->config->type;
	const int changeThreshold = this->config->changeThreshold;

	const bool showPreview = this->_programConfig->showPreview;   
	const bool showProcessedImages = this->_programConfig->showProcessedFrames;

	const int maxRegisterPerPoint = 10;

	// ammount of frame that recognition will be active before going to idle state    
	const uint8_t framesToRecognice = (100 / (interval + 0.0)) * 30; 
		
	cv::VideoCapture capture(this->config->url);

	std::cout << "Opening " << camName << "..." << std::endl;
	assert(capture.isOpened());

	auto timeLastframe = std::chrono::high_resolution_clock::now();
	bool shouldProcessFrame = false;

	// -----------
	//  Messages
	// -----------
	const int secondsBetweenMessages = this->_programConfig->secondsBetweenMessage;

	while (!*this->_stop_flag && capture.isOpened()) {
		const NISTATE camState = this->state;

		// Read a new frame from the capturer
		this->now = std::chrono::high_resolution_clock::now();
		auto intervalFrames = (now - timeLastframe) / std::chrono::milliseconds(1);
		if (intervalFrames >= interval) {
			capture.read(this->frame);
			timeLastframe = std::chrono::high_resolution_clock::now();
			shouldProcessFrame = true;
		} else {            
			capture.read(this->frame); // keep reading to avoid error on VC.
		}

		if (shouldProcessFrame) {
			// If the frame is not valid try resetting the connection with the camera
			if (this->frame.rows == 0) {                
				capture.release();
				capture.open(this->config->url);
				assert(capture.isOpened());
				continue;
			}

			// Once a new frame is ready, update buffer frames
			if (this->_programConfig->useGifInsteadImage) {
				cv::resize(this->frame, this->frame, RESIZERESOLUTION);

				// if still doesn't detect something (or it maybe detected and we still didn't fill the frames before it needed)
				if (this->gifFrames.updateBefore || this->gifFrames.totalFramesBefore < this->_programConfig->numberGifFrames.framesBefore) {
					// increase total (if it's max then just leave it there)
					this->gifFrames.totalFramesBefore += this->gifFrames.totalFramesBefore >= this->_programConfig->numberGifFrames.framesBefore ? 0 : 1;

					// copy frame
					this->frame.copyTo(this->gifFrames.before[this->gifFrames.indexBefore]);

					// update next frame index
					size_t i = this->gifFrames.indexBefore;
					this->gifFrames.indexBefore = (i + 1) >= this->_programConfig->numberGifFrames.framesBefore ? 0 : (i + 1);
				} else if (this->gifFrames.updateAfter) { // change detected... get the following frames					 
					if (this->gifFrames.indexAfter < this->_programConfig->numberGifFrames.framesAfter) {
						this->frame.copyTo(this->gifFrames.after[this->gifFrames.indexAfter]);

						this->gifFrames.indexAfter++;
					} else {
						// once we have all the frames...
						this->gifFrames.state = State::Ready;
						this->gifFrames.updateAfter = false;
					}
				}
			}

			this->ApplyBasicsTransformations();

			if (this->lastFrame.rows > 0) {
				this->CalculateNonZeroPixels();
				this->UpdateThreshold();
			}

			if (totalNonZeroPixels > this->config->changeThreshold * 0.7) {
				std::cout << this->config->cameraName
					<< " Non zero pixels=" << totalNonZeroPixels
					<< " Configured threshold=" << this->config->changeThreshold
					<< " State=" << (int)this->gifFrames.state
					<< std::endl;
			}

			this->lastFrame = this->frame;

			if (this->totalNonZeroPixels > this->config->changeThreshold && this->gifFrames.state == State::Initial) {
				std::cout << "Change detected. Checking..."  << std::endl;

				size_t overlappingFindings = 0;
				// since gif does this (check if change inside an ignored area) for each frame... 
				// only do it if user wants a image				
				if (!this->_programConfig->useGifInsteadImage) {
					this->lastFinding = FindRect(diff);
					
					for (auto &&i : this->config->ignoredAreas) {
						cv::Rect inters = this->lastFinding.rect.boundingRect() & i;
						if (inters.area() >= this->lastFinding.rect.boundingRect().area() * this->config->minPercentageAreaNeededToIgnore) {
							overlappingFindings += 1;
						}
					}
				}

				if (overlappingFindings < this->config->thresholdFindingsOnIgnoredArea) {
					// is valid, send an alert
					this->ChangeTheStateAndAlert(now);
				}
			}
			
			// push a new frame to display.
			if (showPreview && !showProcessedImages) {
				if (this->_programConfig->showIgnoredAreas) { 
					// Draw ignored areas
					for (auto i : this->config->ignoredAreas) {
						i.x += this->config->roi.x;
						cv::rectangle(this->frameToShow, i, cv::Scalar(255,0,255));
					}
				}
				
				this->frames.push_back(std::move(this->frameToShow));
			}

			shouldProcessFrame = false;
		}
	}

	std::cout << "Closed connection with " << camName << std::endl;

	capture.release();
}