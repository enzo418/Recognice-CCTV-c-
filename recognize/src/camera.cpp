#include "camera.hpp"

Camera::Camera(CameraConfiguration& cameraConfig, ProgramConfiguration* programConfig, cv::HOGDescriptor* hog) : config(&cameraConfig), _programConfig(programConfig), _descriptor(hog) {
	if (this->_programConfig->analizeBeforeAfterChangeFrames 
		|| this->_programConfig->telegramConfig.sendGifWhenDetectChange
		|| this->_programConfig->localNotificationsConfig.sendGifWhenDetectChange) {
		this->currentGifFrames = std::make_unique<GifFrames>(_programConfig, config);
		this->OpenVideoWriter();
	}

	this->Connect();
	this->accumulatorThresholds = cameraConfig.minimumThreshold;
	
	this->cameraThread = std::make_unique<std::thread>(&Camera::ReadFramesWithInterval, this);

	// higher interval -> lower max frames & lower interval -> higher max frames
	this->maxFramesLeft = (100 / (programConfig->msBetweenFrameAfterChange)) * 140; // 100 ms => max = 70 frames
	this->numberFramesToAdd = this->maxFramesLeft * 0.1;

	// allocate 100 Mat, each is aprox 0.6 MB (640x360 color) 100 * 0.6 = 60 MB per camera
	this->frames = std::make_unique<moodycamel::ReaderWriterQueue<cv::Mat>>(100);

	// allocate a empty lastFrame to avoid checking if is empty every time
	this->lastFrame = cv::Mat(this->config->roi.size(), CV_8UC1);
}

Camera::~Camera() {}

void Camera::Connect() {
	this->capturer.open(this->config->url);
}

void Camera::OpenVideoWriter() {
    if (this->_programConfig->saveChangeInVideo) {
		// initialize recorder
		int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
		double fps = 25.0;  // framerate of the created video stream
		std::string filename =  "./" + this->_programConfig->imagesFolder + "/" + std::to_string(config->order) + "_" + Utils::GetTimeFormated() + ".avi"; // name of the output video file
		outVideo.open(filename, codec, fps, RESIZERESOLUTION, true);

		// check if we succeeded
		if (!outVideo.isOpened()) {
			std::cerr 	<< "Could not open the output video file for write"
						<< "\n\tFileName: " << filename;
		}
	}
}

void Camera::AppendFrameToVideo(cv::Mat& frame) {
	this->outVideo << frame;
}

void Camera::ReleaseChangeVideo() {
	this->outVideo.release();
	this->OpenVideoWriter();
}

//std::thread Camera::StartDetection() {
//	return ;
//}

// Cuts the image, applies the desired rotation and the converts the image to white and black.
void Camera::ApplyBasicsTransformations() {
	// if use gif is true, it's already resized
	if ((	this->_programConfig->telegramConfig.useTelegramBot 
			|| this->_programConfig->localNotificationsConfig.useLocalNotifications
		)	
		&& !(this->_programConfig->analizeBeforeAfterChangeFrames 
			|| this->_programConfig->telegramConfig.sendGifWhenDetectChange
			|| this->_programConfig->localNotificationsConfig.sendGifWhenDetectChange
			)
		)
		cv::resize(this->frame, this->frame, RESIZERESOLUTION);
	
	this->frameToShow = this->frame.clone();

	// auto intervalFrames = (now - this->lastBackupImageStored) / std::chrono::seconds(1);
	// if (intervalFrames >= 10) {
	// 	this->imageFrom10SecondsAgo = this->imageFrom10SecondsAgo;
	// 	this->lastBackupImageStored = std::chrono::high_resolution_clock::now();
	// }

	// Then rotate it
	ImageManipulation::RotateImage(this->frame, this->config->rotation);

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

		if (this->_programConfig->showIgnoredAreas) { 
			// Draw ignored areas
			for (auto&& i : this->config->ignoredAreas)
				cv::rectangle(d2, i, cv::Scalar(255,0,255));
		}
		
		this->frames->try_enqueue(std::move(d2));
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
	// this->pendingNotifications.push_back(Notification::Notification());

	const bool sendGif = this->_programConfig->telegramConfig.sendGifWhenDetectChange
							|| this->_programConfig->localNotificationsConfig.sendGifWhenDetectChange;
	
	if (sendGif || this->_programConfig->analizeBeforeAfterChangeFrames) {
		this->currentGifFrames->detectedChange();
	}

	if (!this->_programConfig->analizeBeforeAfterChangeFrames) {
		if (this->_programConfig->localNotificationsConfig.sendImageWhenDetectChange
			|| this->_programConfig->telegramConfig.sendImageWhenDetectChange) 
		{
			// Send message with image
			auto intervalFrames = (now - this->lastImageSended) / std::chrono::seconds(1);
			if (intervalFrames >= this->_programConfig->secondsBetweenImage) {
				Notification::Notification imn(this->frameToShow, "Movimiento detectado en esta camara.", true);
				this->pendingNotifications.push_back(imn);
				
				this->lastImageSended = std::chrono::high_resolution_clock::now();
			}
		}
		
		// Send text message
		auto intervalFrames = (now - this->lastTextSended) / std::chrono::seconds(1);
		if (intervalFrames >= this->_programConfig->secondsBetweenMessage 
			&& 	(
					this->_programConfig->localNotificationsConfig.sendTextWhenDetectChange
					|| this->_programConfig->telegramConfig.sendTextWhenDetectChange
				)
			&& !(this->_programConfig->telegramConfig.sendGifWhenDetectChange
				|| this->_programConfig->localNotificationsConfig.sendGifWhenDetectChange)/* If is using gif then don't send since, it will send text if the gif is valid*/
			)
		{
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

	const char* camName = &this->config->cameraName[0];

	const bool showPreview = this->_programConfig->showPreview;   
	const bool showProcessedImages = this->_programConfig->showProcessedFrames;
	const bool showIgnoredAreas = this->_programConfig->showIgnoredAreas;
	const bool useNotifications = this->_programConfig->telegramConfig.useTelegramBot 
								  || this->_programConfig->localNotificationsConfig.useLocalNotifications;
	const bool useGif = this->_programConfig->telegramConfig.sendGifWhenDetectChange
						|| this->_programConfig->localNotificationsConfig.sendGifWhenDetectChange;

	cv::VideoCapture capture(this->config->url);

	std::cout << "Opening " << camName << "..." << std::endl;
	assert(capture.isOpened());

	auto timeLastframe = std::chrono::high_resolution_clock::now();
	bool shouldProcessFrame = false;

	// -----------
	//  Messages
	// -----------
	const int secondsBetweenMessages = this->_programConfig->secondsBetweenMessage;

	while (!this->close && capture.isOpened()) {
		// Read a new frame from the capturer
		this->now = std::chrono::high_resolution_clock::now();
		auto intervalFrames = (now - timeLastframe) / std::chrono::milliseconds(1);
		if (intervalFrames >= this->msBetweenFrames) {
			capture.read(this->frame);
			timeLastframe = std::chrono::high_resolution_clock::now();
			shouldProcessFrame = true;
		} else {            
			capture.read(this->frame); // keep reading to avoid error on VC.
		}

		if (shouldProcessFrame) {
			// If the frame is not valid try resetting the connection with the camera
			// if (this->frame.rows == 0) {                
			// 	capture.release();
			// 	capture.open(this->config->url);
			// 	assert(capture.isOpened());
			// 	continue;
			// }

			// Once a new frame is ready, update buffer frames
			if (useNotifications && this->_programConfig->analizeBeforeAfterChangeFrames || useGif) {
				cv::resize(this->frame, this->frame, RESIZERESOLUTION);

				// if the current gif is ready to be sent, move it to the vector of gifs ready
				// and create another gif
				if (this->currentGifFrames->getState() == State::Ready && this->gifsReady.size() < 10) {
					this->gifsReady.push_back(std::move(this->currentGifFrames));

					this->currentGifFrames = std::make_unique<GifFrames>(
						this->_programConfig, 
						this->config						
					);
					std::cout << "[N] Pushed a new gif." << std::endl;
				} else if (this->gifsReady.size() >= 5) {
					std::cout << "[N] Gifs ready reached max value." << std::endl;
				}

				this->currentGifFrames->addFrame(this->frame);
			}

			this->ApplyBasicsTransformations();

			this->CalculateNonZeroPixels();
			this->UpdateThreshold();

			// if (totalNonZeroPixels > this->config->changeThreshold * 0.7) {
			// 	std::cout << this->config->cameraName
			// 		<< " Non zero pixels=" << totalNonZeroPixels
			// 		<< " Configured threshold=" << this->config->changeThreshold
			// 		<< " State=" << (int)this->currentGifFrames->getState()
			// 		<< std::endl;
			// }

			this->lastFrame = this->frame;

			if (this->totalNonZeroPixels > this->config->changeThreshold 
				&& this->currentGifFrames->getState() == State::Initial) 
			{
				if (useNotifications) {
					std::cout << "Change detected. Checking... Threshold: " << this->totalNonZeroPixels  << std::endl;

					this->msBetweenFrames = this->_programConfig->msBetweenFrameAfterChange;

					size_t overlappingFindings = 0;
					// since gif does this (check if change inside an ignored area) for each frame... 
					// only do it if user wants a image				
					if (!this->_programConfig->analizeBeforeAfterChangeFrames && !useGif) {
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
					
					if (framesLeft < maxFramesLeft)
						framesLeft += numberFramesToAdd;
				}
			}

			if (framesLeft == 0)
				this->msBetweenFrames = this->_programConfig->msBetweenFrame;
			
			// push a new frame to display.
			if (showPreview && !showProcessedImages) {
				if (showIgnoredAreas) { 
					// Draw ignored areas
					for (auto&& i : this->config->ignoredAreas) {
						i.x += this->config->roi.x;
						cv::rectangle(this->frameToShow, i, cv::Scalar(255,0,255));
					}
				}
				
				this->frames->try_enqueue(std::move(this->frameToShow));  // Will only succeed if the queue has an empty slot (never allocates)
			}

			shouldProcessFrame = false;
			framesLeft--;
		}
	}

	std::cout << "Closed connection with " << camName << std::endl;

	capture.release();
}