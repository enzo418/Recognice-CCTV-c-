#include "Camera.hpp"

Camera::Camera(CameraConfiguration cameraConfig, ProgramConfiguration* programConfig, bool* stopFlag, cv::HOGDescriptor* hog) : config(cameraConfig), _programConfig(programConfig), _stop_flag(stopFlag), _descriptor(hog) {
	this->gifFrames.after.resize(*programConfig->numberGifFrames.framesAfter);
	this->gifFrames.before.resize(*programConfig->numberGifFrames.framesBefore);
	this->Connect();
	this->accumulatorThresholds = cameraConfig.changeThreshold;
}

void Camera::Connect() {
	this->capturer.open(this->config.url);
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

	// Take the region of interes
	if (!this->config.roi.isEmpty()) {
		cv::Rect roi(this->config.roi.point1, this->config.roi.point2);
		this->frame = this->frame(roi);
	}

	// Then rotate it
	if (this->config.rotation != 0) ImageManipulation::RotateImage(this->frame, this->config.rotation);

	cv::cvtColor(this->frame, this->frame, cv::COLOR_RGB2GRAY);

	// Don't use hight constrast here... it only increases the change between images.
	// is more usefull when trying to detect objs.
}

void Camera::CalculateNonZeroPixels() {
	cv::absdiff(this->lastFrame, frame, diff);

	cv::GaussianBlur(diff, diff, cv::Size(3, 3), 10);

	// 45 works perfect with most of the cameras/resolution
	cv::threshold(diff, diff, this->config.noiseThreshold, 255, cv::THRESH_BINARY);

	totalNonZeroPixels = cv::countNonZero(diff);

	if (this->_programConfig->showProcessedFrames && this->_programConfig->showPreview) {
		cv::Mat d2;
		// place diff image on top the frame img
		cv::addWeighted(frame, 1, diff, 8, 12, d2);
		
		this->frames.push_back(d2);
	}
}

void Camera::UpdateThreshold(){
	if(this->totalNonZeroPixels > 0){
		this->accumulatorThresholds += this->config.minimumThreshold + this->totalNonZeroPixels;
		// std::cout << "+" << this->totalNonZeroPixels << " total=" << this->accumulatorThresholds << std::endl;
		this->thresholdSamples++;
	}
		
	if(this->thresholdSamples == 0 || this->accumulatorThresholds == 0) return;

	auto time = (this->now - this->lastThresholdUpdate) / std::chrono::seconds(1);
	if(time >= this->config.updateThresholdFrequency) {
		const int oldThres = this->config.changeThreshold;

		this->config.changeThreshold = this->accumulatorThresholds / this->thresholdSamples * this->config.increaseTresholdFactor;

		std::cout << "[I] Threshold " << this->config.cameraName 
				<< " freq=" << this->config.updateThresholdFrequency
				<< " chaged from: " << oldThres << " to: " << this->config.changeThreshold 
				<< " [accumulator=" << accumulatorThresholds
				<< " / samples=" << this->thresholdSamples
				<< "] => average=" << this->accumulatorThresholds / this->thresholdSamples
				<< " (factor = " << this->config.increaseTresholdFactor << ")"
				<< std::endl;
			
		this->accumulatorThresholds = 0;
		this->thresholdSamples = 0;

		this->lastThresholdUpdate = std::chrono::high_resolution_clock::now();
	}
}

void Camera::ChangeTheStateAndAlert(std::chrono::system_clock::time_point& now) {
	if (this->config.state != NI_STATE_DETECTED && this->config.type != CAMERA_SENTRY && this->config.state != NI_STATE_DETECTING)
		this->config.state = NI_STATE_DETECTING; // Change the state of the camera
					
	// every secondsBetweenMessages send a message to the telegram bot
	auto intervalFrames = (now - this->lastMessageSended) / std::chrono::seconds(1);
	if (intervalFrames >= this->_programConfig->secondsBetweenMessage) {	
		// Play a sound
		this->pendingNotifications.push_back(Notification::Notification());

		if(this->_programConfig->sendImageWhenDetectChange && !this->_programConfig->useGifInsteadImage){                  
			Notification::Notification imn(this->frameToShow, "Movimiento detectado en esta camara.", false);
			this->pendingNotifications.push_back(imn);
		} else if (this->_programConfig->sendImageWhenDetectChange && this->_programConfig->useGifInsteadImage) {			
			this->gifFrames.updateBefore = false;
			this->gifFrames.updateAfter = true;
		} else {
			Notification::Notification imn("Movimiento detectado en la camara " + this->config.cameraName);
			this->pendingNotifications.push_back(imn);
		}

		this->lastMessageSended = std::chrono::high_resolution_clock::now();
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

	const char* camName = &this->config.cameraName[0];
	const CAMERATYPE camType = this->config.type;
	const int changeThreshold = this->config.changeThreshold;

	const bool showPreview = this->_programConfig->showPreview;   
	const bool showProcessedImages = this->_programConfig->showProcessedFrames;

	const int maxRegisterPerPoint = 10;

	// ammount of frame that recognition will be active before going to idle state    
	const uint8_t framesToRecognice = (100 / (interval + 0.0)) * 30; 
		
	cv::VideoCapture capture(this->config.url);

	std::cout << "Opening " << camName << "..." << std::endl;
	assert(capture.isOpened());

	auto timeLastframe = std::chrono::high_resolution_clock::now();
	bool shouldProcessFrame = false;
	
	// -----------
	//  Messages
	// -----------
	const int secondsBetweenMessages = this->_programConfig->secondsBetweenMessage;

	while (!*this->_stop_flag && capture.isOpened()) {
		const NISTATE camState = this->config.state;

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
				capture.open(this->config.url);
				assert(capture.isOpened());
				continue;
			}

			// Once a new frame is ready, update gif images
			if (this->_programConfig->useGifInsteadImage) {
				cv::resize(this->frame, this->frame, RESIZERESOLUTION);

				if (this->gifFrames.updateBefore || this->gifFrames.totalFramesBefore < *this->_programConfig->numberGifFrames.framesBefore) {
					this->gifFrames.totalFramesBefore += this->gifFrames.totalFramesBefore >= *this->_programConfig->numberGifFrames.framesBefore ? 0 : 1;

					this->frame.copyTo(this->gifFrames.before[this->gifFrames.indexBefore]);

					size_t i = this->gifFrames.indexBefore;
					this->gifFrames.indexBefore = (i + 1) >= *this->_programConfig->numberGifFrames.framesBefore ? 0 : (i + 1);
				} else if (this->gifFrames.updateAfter) {
					if (this->gifFrames.indexAfter < *this->_programConfig->numberGifFrames.framesAfter) {
						this->frame.copyTo(this->gifFrames.after[this->gifFrames.indexAfter]);

						this->gifFrames.indexAfter++;
					} else {
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

			if (totalNonZeroPixels > this->config.changeThreshold * 0.7) {
				std::cout << this->config.cameraName
					<< " Non zero pixels=" << totalNonZeroPixels
					<< " Configured threshold=" << this->config.changeThreshold
					<< std::endl;
			}

			this->lastFrame = this->frame;

			if (this->totalNonZeroPixels > this->config.changeThreshold) {				
				std::cout << "Diff frame saved for later."  << std::endl;

				size_t overlappingFindings = 0;
				// since gif does this for each frame...
				if (!this->_programConfig->useGifInsteadImage){
					this->lastFinding = FindRect(diff);
					
					for (auto &&i : this->config.ignoredAreas) {
						cv::Rect inters = this->lastFinding.rect.boundingRect() & i;
						if (inters.area() >= this->lastFinding.rect.boundingRect().area() * this->minPercentageAreaNeededToIgnore) {
							overlappingFindings += 1;
						}
					}
				}

				if (overlappingFindings < this->thresholdFindingsOnIgnoredArea) {
					// is valid, send an alert
					this->ChangeTheStateAndAlert(now);

					// Increment frames left
					if (framesLeft < maxFramesLeft)
						framesLeft += framesToRecognice;
				}
			}

			// camera type sentry = no detection, only seeks for changes in the image
			if (camType == CAMERA_SENTRY) {
				if (framesLeft > 0) {
					if (camState != NI_STATE_DETECTING) {
						this->config.state = NI_STATE_DETECTING;
					}
					framesLeft--;
				} else {
					this->config.state = NI_STATE_SENTRY;					
				}

				shouldProcessFrame = false;
				
				// push a new frame to display.
				if (showPreview && !showProcessedImages)
					this->frames.push_back(this->frameToShow);					
			} else {
				// camera type active => tries to detect a person.

				if (framesLeft > 0) {					
					framesLeft--;
				}

				if (framesLeft == 0) {
					this->config.state = NI_STATE_SENTRY;
				}

				// push a new frame to display.
				if (showPreview && !showProcessedImages) {
					// Draw ignored areas
					// for (auto i : this->config.ignoredAreas) {						
					// 	i.x += this->config.roi.point1.x;						
					// 	cv::rectangle(this->frameToShow, i, cv::Scalar(255,0,255));
					// }
					
					this->frames.push_back(std::move(this->frameToShow));
				}
			}

			shouldProcessFrame = false;
		}
	}

	std::cout << "Closed connection with " << camName << std::endl;

	capture.release();
}