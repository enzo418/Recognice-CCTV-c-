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

	this->msBetweenFrames = this->_programConfig->msBetweenFrame;
}

Camera::~Camera() {}

void Camera::Connect() {
	this->capturer.open(this->config->url);
}

void Camera::OpenVideoWriter(bool overwriteLastVideo) {
    if (this->_programConfig->saveChangeInVideo) {
		// initialize recorder
		// int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');  // select desired codec (must be available at runtime)
		int codec = cv::VideoWriter::fourcc('H', '2', '6', '4');
		double fps = 8.0;  // framerate of the created video stream
		
		std::cout << "\n[V] Files before: " 
					<< "[" << (int)currentIndexVideoPath << "] " << videosPath[currentIndexVideoPath] << ", " 
					<< "[" << (int)!currentIndexVideoPath << "] " << videosPath[!currentIndexVideoPath]
					<< std::endl;

		currentIndexVideoPath = !currentIndexVideoPath;
		if (videosPath[currentIndexVideoPath].length() == 0 || !overwriteLastVideo) {		
			videosPath[currentIndexVideoPath] = this->_programConfig->imagesFolder + "/" + std::to_string(this->config->order) + "_" + std::to_string(clock()) + ".mp4"; // name of the output video file
		}

		std::cout << "\n[V] Files Now: " 
					<< "[" << (int)currentIndexVideoPath << "] " << videosPath[currentIndexVideoPath] << ", " 
					<< "[" << (int)!currentIndexVideoPath << "] " << videosPath[!currentIndexVideoPath]
					<< "\n\n";
		
		outVideo.open(videosPath[currentIndexVideoPath], codec, fps, RESIZERESOLUTION, true);

		std::cout << "Created video output. FPS=" << fps<< std::endl;

		// check if we succeeded
		if (!outVideo.isOpened()) {
			std::cerr 	<< "Could not open the output video file for write"
						<< "\n\tFileName: " << videosPath[currentIndexVideoPath]
						<< std::endl;
		}

		this->lastVideoStartTime = this->now;
	}
}

void Camera::AppendFrameToVideo(cv::Mat& frame) {
	this->outVideo << frame;
}

void Camera::ReleaseChangeVideo() {
	this->outVideo.release();
}

void Camera::ReleaseAndOpenChangeVideo(bool overwriteLastVideo) {
	this->outVideo.release();
	this->OpenVideoWriter(overwriteLastVideo);
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
	} else {
		if (this->_programConfig->localNotificationsConfig.sendImageWhenDetectChange
			|| this->_programConfig->telegramConfig.sendImageWhenDetectChange) 
		{
			// Send message with image
			auto intervalFrames = (now - this->lastImageSended) / std::chrono::seconds(1);
			if (intervalFrames >= this->_programConfig->secondsBetweenImage) {
				this->pendingNotifications.push_back(
					Notification::Notification(
						this->frameToShow, 
						Utils::FormatNotificationTextString(
							this->_programConfig->messageOnTextNotification, 
							this->config->cameraName
							),
						true,
						0)
					// Notification::Notification::Image(
					// 	this->frameToShow, 
					// 	Utils::FormatNotificationTextString(
					// 		this->_programConfig->messageOnTextNotification, 
					// 		this->config->cameraName
					// 		),
					// 	true,
					// 	0)
				);
				
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
			&& /* If is using gif then don't send since, it will send text if the gif is valid*/
				!(this->_programConfig->telegramConfig.sendGifWhenDetectChange
				|| this->_programConfig->localNotificationsConfig.sendGifWhenDetectChange)
			)
		{
			this->pendingNotifications.push_back(
				Notification::Notification(
					Utils::FormatNotificationTextString(
						this->_programConfig->messageOnTextNotification, 
						this->config->cameraName
					),
					0
				)
				// Notification::Notification::Text(
				// 	Utils::FormatNotificationTextString(
				// 		this->_programConfig->messageOnTextNotification, 
				// 		this->config->cameraName
				// 	),
				// 	0
				// )
			);
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
	
	const bool sendVideo = this->_programConfig->telegramConfig.sendVideoWhenDetectChange
						|| this->_programConfig->localNotificationsConfig.sendVideoWhenDetectChange;

	const bool saveChangeVideo = this->_programConfig->saveChangeInVideo;

	// this is the max length of the full ouput video of the change,
	// doesn't mean it will be exactly <maxVideoMinutesLength> minutes long.
	const int maxVideoLengthSeconds = 20;
	
	// Split the max length by 2 since we use 2 videos
	const double singleVideoMaxSecondsLength = maxVideoLengthSeconds / 2;

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
			if (this->frame.rows == 0) {
				capture.release();
				capture.open(this->config->url);
				
				// TODO: PUSH ERROR TO VECTOR OF RECOGNIZE
				assert(capture.isOpened());
				continue;
			}

			// Once a new frame is ready, update buffer frames
			if (useNotifications && this->_programConfig->analizeBeforeAfterChangeFrames || useGif || sendVideo) {
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
						this->videoLocked = true;
					}
					
					if (framesLeft < maxFramesLeft)
						framesLeft += numberFramesToAdd;
				}
			} else if (!this->videoLocked && saveChangeVideo) {
				auto secondDiff = std::chrono::duration_cast<std::chrono::seconds>(this->now - this->lastVideoStartTime).count();
				if (secondDiff >= singleVideoMaxSecondsLength) {
					std::cout 	<< "\n[V] Video " << videosPath[currentIndexVideoPath] 
								<< " has " << secondDiff << " of length, " << secondDiff - singleVideoMaxSecondsLength
								<< "s more than needed. Send video continuation? " 
								<< (this->sendChangeVideoContinuation ? "Yes" : "No") << "\n\n";
					// if (this->sendChangeVideoContinuation /**&&  Wants continuation video? */) {
					// 	std::string fileName = this->videosPath[this->currentIndexVideoPath];
					// 	this->ReleaseAndOpenChangeVideo(false);
					// 	this->pendingNotifications.push_back(Notification::Notification(fileName, camName, this->continuation_group_id));

					// 	this->sendChangeVideoContinuation = false;
					// 	this->continuation_group_id = 0;
					// } else {
					// 	std::cout << "[V] Relased video " <<  (int)currentIndexVideoPath 
					// 	<< ". Overwriting video " << (int)!currentIndexVideoPath << std::endl;

					// 	this->ReleaseAndOpenChangeVideo(true);
					// }
				}
			}

			if (framesLeft == 0)
				this->msBetweenFrames = this->_programConfig->msBetweenFrame;
			
				
			if (saveChangeVideo)
				outVideo.write(this->frameToShow);

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

	outVideo.release();
	capture.release();
}