#include "recognize.hpp"

Recognize::Recognize() {
	this->hogDescriptor.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
}

std::vector<cv::Mat*> Recognize::AnalizeLastFramesSearchBugs(Camera& camera) {
	camera.gifFrames.state = State::Wait;
	
	const size_t ammountOfFrames = programConfig.numberGifFrames.framesBefore + programConfig.numberGifFrames.framesAfter;
	std::vector<FrameDescriptor> framesTransformed(ammountOfFrames);
	std::vector<cv::Mat*> frames(ammountOfFrames);
	
	cv::Mat* frameCero;
	cv::Mat diff;
	
	size_t validFrames = 0;
	double totalDistance = 0;
	double totalArea = 0;
	double totalNonPixels = 0;
	size_t overlappingFindings = 0;

	cv::Point p1;
	cv::Point p2;

	FindingInfo* lastValidFind = nullptr;

	//// First get all the frames in order

	size_t totalFrames = 0;
	for (size_t i = camera.gifFrames.indexBefore;;) {	
		if (totalFrames < programConfig.numberGifFrames.framesBefore) {						
			frames[totalFrames] = &camera.gifFrames.before[i];

			// Take the region of interes
			if (!camera.config->roi.empty()) {
				framesTransformed[totalFrames].frame = (*frames[totalFrames]).clone();
				framesTransformed[totalFrames].frame = framesTransformed[totalFrames].frame(camera.config->roi);
			}

			if (camera.config->rotation != 0) ImageManipulation::RotateImage(framesTransformed[totalFrames].frame, camera.config->rotation);

			if (totalFrames == 0) {
				frameCero = &framesTransformed[0].frame;
			}

			totalFrames++;
			i = (i + 1) >= programConfig.numberGifFrames.framesBefore ? 0 : (i + 1);
		} else
			break;
	}
	
	for (; totalFrames < ammountOfFrames; totalFrames++) {
		frames[totalFrames] = &camera.gifFrames.after[totalFrames - programConfig.numberGifFrames.framesBefore];
		
		if (!camera.config->roi.empty()) {
			framesTransformed[totalFrames].frame = (*frames[totalFrames]).clone(); 
			framesTransformed[totalFrames].frame = framesTransformed[totalFrames].frame(camera.config->roi); 
		}

		if (camera.config->rotation != 0) ImageManipulation::RotateImage(framesTransformed[totalFrames].frame, camera.config->rotation);
	}

	//// Process frames

	bool p1Saved = false;
	for (size_t i = 1; i < frames.size(); i++) {		
		cv::absdiff(*frameCero, framesTransformed[i].frame, diff);
		cv::GaussianBlur(diff, diff, cv::Size(3, 3), 10);

		cv::threshold(diff, diff, camera.config->noiseThreshold, 255, cv::THRESH_BINARY);

		cv::cvtColor(diff, diff, cv::COLOR_BGR2GRAY);

		FindingInfo finding = FindRect(diff);
		framesTransformed[i].finding = finding;
			
		totalNonPixels += cv::countNonZero(diff);

		if (finding.isGoodMatch) {
			if (!p1Saved) {
				p1 = finding.center;
				p1Saved = true;
			}

			cv::Point2f vertices[4];
			finding.rect.points(vertices);
			for (int j = 0; j < 4; j++) {			
				vertices[j].x += camera.config->roi.x;
			}
			
			// check if finding is overlapping with a ignored area
			for (auto &&i : camera.config->ignoredAreas) {					
				cv::Rect inters = finding.rect.boundingRect() & i;
				if (inters.area() >= finding.rect.boundingRect().area() * camera.config->minPercentageAreaNeededToIgnore) {
					overlappingFindings += 1;
				}
			}
			
			for (int j = 0; j < 4; j++) {			
				cv::line(*frames[i], vertices[j], vertices[(j+1)%4], cv::Scalar(255,255,170), 1);
			}

			if (lastValidFind != nullptr) {
				validFrames++;
				totalDistance += euclideanDist(framesTransformed[i].finding.center, lastValidFind->center);
				totalArea += abs(framesTransformed[i].finding.area - lastValidFind->area);
			}
		
			lastValidFind = &framesTransformed[i].finding;

			p2 = finding.center;
		}
	}

	double displacementX = abs(p1.x - p2.x);
	double displacementY = abs(p1.y - p2.y);

	if (camera.gifFrames.avrgDistanceFrames > 25)
		camera.gifFrames.debugMessage += "\nIGNORED";

	camera.gifFrames.debugMessage += "\ntotalNonPixels: " + std::to_string(totalNonPixels) + " totalArea: " + std::to_string(totalArea) + " total area % of non zero: " + std::to_string(totalArea * 100 / totalNonPixels);
	camera.gifFrames.debugMessage += "\nP1: [" + std::to_string(p1.x) + "," + std::to_string(p1.y) + "] P2: [" + std::to_string(p2.x) + "," + std::to_string(p2.y) + "] Distance: " + std::to_string(euclideanDist(p1, p2)) + "\n DisplX: " + std::to_string(displacementX) + " DisplY: " + std::to_string(displacementY);

	if (validFrames != 0) {
		camera.gifFrames.avrgDistanceFrames = totalDistance / validFrames;
		camera.gifFrames.avrgAreaDifference = totalArea / validFrames;
	}

	//// Check if it's valid
	if (validFrames != 0 && camera.gifFrames.avrgDistanceFrames <= 25 && overlappingFindings < camera.config->thresholdFindingsOnIgnoredArea) {
		//// Recognize a person
		bool personDetected = false;
		size_t start = programConfig.numberGifFrames.framesBefore - camera.config->framesToAnalyze.framesBefore;
		start = start < 0 ? 0 : start;

		size_t end = programConfig.numberGifFrames.framesAfter + camera.config->framesToAnalyze.framesAfter;
		end = end > frames.size() ? frames.size() : end;

		if (camera.config->type == CAMERA_ACTIVE) {
			for (size_t i = start; i < end; i++) {
				std::vector<cv::Rect> detections;
				std::vector<double> foundWeights;

				// query descriptor with frame
				this->hogDescriptor.detectMultiScale(framesTransformed[i].frame, detections, foundWeights, camera.config->hitThreshold, cv::Size(8, 8), cv::Size(4, 4), 1.05);
				size_t detectSz = detections.size();			

				if(detectSz > 0) {
					// draw detections on frame
					for (size_t i = 0; i < detectSz; i++) {
						// detections[i].x += camera.config->roi.x;
						cv::Scalar color = cv::Scalar(0, foundWeights[i] * foundWeights[i] * 200, 0);					
						cv::rectangle(framesTransformed[i].frame, detections[i], color);					
						// putText(frame, std::to_string(foundWeights[i]), Utils::BottomRightRectangle(detections[i]), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));
					}

					camera.state = NI_STATE_DETECTED;

					// send a extra notification?
					// Notification::Notification ntf (*frames[i], "Se ha detectado algo en esta camara.", true);
					// this->pendingNotifications.push_back(ntf);
					camera.gifFrames.debugMessage += "\nA person was detected.";
					
					personDetected = true;
					break;
				}
			}
		}

		/// TODO: Add config to select if want to send only when someone is detected
		// if (personDetected) {
			camera.gifFrames.state = State::Send;
		// } else {
		// 	camera.gifFrames.state = State::Cancelled;
		// }
	} else {		
		camera.gifFrames.state = State::Cancelled;
		camera.gifFrames.debugMessage += "\n";

		// This if is only for debuggin purposes
		if (validFrames != 0) {
			camera.gifFrames.avrgDistanceFrames = 0;
			camera.gifFrames.avrgAreaDifference = 0;
			camera.gifFrames.debugMessage += "\nCancelled due 0 valid frames found.";
		} else {
			camera.gifFrames.debugMessage += "\nCancelled due to overlapping with ignored areas: " +  std::to_string(overlappingFindings) + " of " + std::to_string(camera.config->thresholdFindingsOnIgnoredArea) + " validFrames needed.";
		}
	}

	camera.gifFrames.debugMessage += "\nAverage distance between 2 frames finding: " + std::to_string(camera.gifFrames.avrgDistanceFrames) + "\naverage area difference: " + std::to_string(camera.gifFrames.avrgAreaDifference) + "\n frames used: " + std::to_string(validFrames) + "\n overlapeds: " + std::to_string(overlappingFindings);
	
	std::cout << camera.gifFrames.debugMessage << std::endl;

	return frames;
}

void Recognize::Start(Configurations& configs, bool startPreviewThread, bool startActionsThread) {	
	this->close = false;
	this->stop = false;
	
	this->programConfig = configs.programConfig;
	this->camerasConfigs = configs.camerasConfigs;

	// Remove disabled cameras
	size_t sz = this->camerasConfigs.size();
	for (size_t i = 0; i < sz; i++)
		if (this->camerasConfigs[i].type == CAMERA_DISABLED || this->camerasConfigs[i].url.empty()) {
			this->camerasConfigs.erase(this->camerasConfigs.begin() + i);
			i--;
			sz--;
		}
	
	if (this->camerasConfigs.size() == 0) {
		/// TODO: throw custom exception
		std::cout << "Cameras size 0. Exiting." << std::endl;
		std::exit(-1);
	}
	
	this->indexMainThreadCameras = this->threads.size();
	std::cout << "pushed thread of cameras in " << this->threads.size() << std::endl;
	this->threads.push_back(std::thread(&Recognize::StartCamerasThreads, this));

	if (startPreviewThread) {
		std::cout << "pushed thread of preview in " << this->threads.size() << std::endl;
		// Start the thread to show the images captured.
		this->threads.push_back(std::thread(&Recognize::StartPreviewCameras, this));
	}

	if (startActionsThread) {
		std::cout << "pushed thread of actions bot in " << this->threads.size() << std::endl;
		this->threads.push_back(std::thread(&Recognize::StartActionsBot, this));
	}

	if (programConfig.telegramConfig.useTelegramBot) {
		std::cout << "pushed thread of notifications in " << this->threads.size() << std::endl;		
		// Start a thread for save and upload the images captured    
		this->threads.push_back(std::thread(&Recognize::StartNotificationsSender, this));
	}
}

void Recognize::StartCamerasThreads() {
	bool somethingDetected = false;

	Utils::FixOrderCameras(this->camerasConfigs);
	
	// Create the cameras objs
	for (auto &config : this->camerasConfigs) {
		this->cameras.push_back(Camera(config, &programConfig, &this->stop, &this->hogDescriptor));
	}

	// Start a thread for each camera
	for (size_t i = 0; i < this->cameras.size(); i++) {
		this->threads.push_back(this->cameras[i].StartDetection());
	}
}

void Recognize::StartActionsBot() {
	auto lastCheck = std::chrono::high_resolution_clock::now();
	auto now = std::chrono::high_resolution_clock::now();
	std::string lastMessage = "";
	std::string message = "";
	std::time_t unix_time_start = std::time(0);
	
	while (!close) {
		now = std::chrono::high_resolution_clock::now();
		auto diff = (now - lastCheck) / std::chrono::seconds(1);
		if(diff >= 9) {
			std::time_t unix_time = 0;
			std::string fromId = TelegramBot::GetLastMessageFromBot(programConfig.telegramConfig.apiKey, message, unix_time, programConfig.authUsersToSendActions);
			std::string reply = "Comando no reconocido.";

			if(unix_time > unix_time_start) {
				std::cout << "Message: " << message << " Last Message: " << lastMessage << std::endl;
				
				if(lastMessage != message){
					/// TODO: Read the messages needed from a file so is user-specific.
					if (message == "/apagar") {
						close = true; // closes the bot
						stop = true; // closes the cameras connections

						reply = "Reconocedor apagado.";
					} else if (message == "/pausar"){
						stop = true;

						if(this->threads[this->indexMainThreadCameras].joinable())
							this->threads[this->indexMainThreadCameras].join();

						cv::destroyAllWindows();

						reply = "Reconocedor pausado.";
					} else if (message == "/reanudar") {
						stop = false;
						this->threads[this->indexMainThreadCameras] = std::thread(&Recognize::StartCamerasThreads, this); 

						reply = "Reconocedor reanudado.";
					}

					TelegramBot::SendMessageToChat(reply, fromId, programConfig.telegramConfig.apiKey);
					lastMessage = message;
				}
			}

			lastCheck = std::chrono::high_resolution_clock::now();
		}

		std::this_thread::sleep_for(std::chrono::seconds(3));		
	}	
}

void Recognize::StartPreviewCameras() {
    // ============
	//  Variables 
	// ============
	  
	size_t amountCameras = this->camerasConfigs.size();

	// saves the frames to show in a iteration
	std::vector<cv::Mat> frames;

	const ushort interval = programConfig.msBetweenFrame;
	
	// saves the cameras that are ready to be displayed
	std::vector<bool> ready;

	// Windows notification alert message and title
	const bool showAreaCameraSees = programConfig.showAreaCameraSees;
	const bool showProcessedFrames = programConfig.showProcessedFrames;
	
	// counts the cameras displayed
	uint8_t size = 0; 

	uint8_t stackHSize = amountCameras > 1 ? 2 : 1;

	// resolution of each frame
	cv::Mat res;

	// saves the current state of the notification icon
	NISTATE currentState = NI_STATE_SENTRY;

	// used to, in case is false, display the last frame of the camera that has no frame
	// in the moment of the iteration.
	bool isFirstIteration = true;
	frames.resize(amountCameras);

	// init vector of cameras ready
	for (size_t i = 0; i < amountCameras; i++) {
		ready.push_back(false);
	}

	ready.shrink_to_fit();

	/* Image saver */
	auto timeLastSavedImage = high_resolution_clock::now();
	ushort secondsBetweenImage = 2;

	// ============
	//  Main loop 
	// ============

	cv::namedWindow("Preview Cameras");

	while (!stop && programConfig.showPreview) {
		if (this->cameras.size() == amountCameras) {
			// if all cameras are in sentry state
			bool allCamerasInSentry = true;

			for (size_t i = 0; i < amountCameras; i++) {
				if (cameras[i].frames.size() > 0) {
					// if the vector pos i has no frame
					if (!ready[cameras[i].config->order]) {
						// take the first frame and delete it
						frames[cameras[i].config->order] = cameras[i].frames[0];

						if (showAreaCameraSees && !showProcessedFrames) {
							cv::Scalar color = cv::Scalar(255, 0, 0);
							cv::rectangle(frames[cameras[i].config->order], cameras[i].config->roi, color);
						}

						cameras[i].frames.erase(cameras[i].frames.begin());
						
						ready[cameras[i].config->order] = true;
						
						size++;
					}
				}
			}
			
			if (size == amountCameras || !isFirstIteration) {
				res = ImageManipulation::StackImages(&frames[0], amountCameras, stackHSize);
				size = 0;
  				
				if (isFirstIteration) {
					cv::startWindowThread();
					isFirstIteration = false;
				}

				for (size_t i = 0; i < amountCameras; i++)
					ready[i] = false;  

				double scaleW = programConfig.outputResolution.width * programConfig.ratioScaleOutput;
				double scaleH = programConfig.outputResolution.height * programConfig.ratioScaleOutput;
				if (!programConfig.outputResolution.empty()){
					scaleW = programConfig.outputResolution.width * programConfig.ratioScaleOutput;
					scaleH = programConfig.outputResolution.height * programConfig.ratioScaleOutput;
				} else {
					scaleW = res.cols * programConfig.ratioScaleOutput;
					scaleH = res.rows * programConfig.ratioScaleOutput;
				}

				cv::resize(res, res, cv::Size(scaleW, scaleH));

				cv::imshow("Preview Cameras", res);
				cv::waitKey(20);            
				programConfig.frameWithAllTheCameras = std::move(res);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(int(interval * 0.7)));
	}  

	cv::destroyWindow("Preview Cameras");
}

void Recognize::StartNotificationsSender() {
	cv::Mat frame;
	std::string date;

	while (!stop) {
		for (auto &&camera : cameras) {			
			// Send gif
			if (programConfig.useGifInsteadImage && camera.gifFrames.state == State::Ready) {
				// -----------------------------------------------------------
				// Take before and after frames and combine them into a .gif
				// -----------------------------------------------------------
				const std::string identifier = std::to_string(clock());
				const std::string imageFolder = programConfig.imagesFolder;
				const std::string root = "./" + imageFolder + "/" + identifier;
				const std::string gifPath = root + ".gif";
				std::string location;
				const size_t gframes = programConfig.numberGifFrames.framesAfter + programConfig.numberGifFrames.framesBefore;

				std::vector<cv::Mat*> frames = this->AnalizeLastFramesSearchBugs(camera);

				if (camera.gifFrames.state == State::Send) {
					// if (frames.avrgDistanceFrames > 70) {
					for (size_t i = 0; i < frames.size(); i++) {
						location = root + "_" + std::to_string((int)i) + ".jpg";

						cv::imwrite(location, *frames[i]);
					}

					std::string command = "convert -resize " + std::to_string(programConfig.gifResizePercentage) + "% -delay 23 -loop 0 " + root + "_{0.." + std::to_string(gframes-1) + "}.jpg " + gifPath;

					std::system(command.c_str());

					TelegramBot::SendMediaToChat(gifPath, "Movimiento detectado. " + camera.gifFrames.debugMessage, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey, true);
				}
				
				// ---
				// update gif collection data
				// ---
				camera.gifFrames.indexBefore = 0;
				camera.gifFrames.indexAfter = 0;
				camera.gifFrames.totalFramesBefore = 0;

				camera.gifFrames.debugMessage = "";
				camera.gifFrames.state = State::Initial;
				camera.gifFrames.updateAfter = false;
				camera.gifFrames.updateBefore = true;
			}

			size_t size = camera.pendingNotifications.size();
			for (size_t i = 0; i < size; i++) {	
				std::cout << "Sending notification of type " << camera.pendingNotifications[i].type << std::endl;

				camera.pendingNotifications[i].send(programConfig);
			}

			// This proc shouldn't clear all the notifcations since it's a multithread process :p
			camera.pendingNotifications.clear();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	}
}

void Recognize::CloseAndJoin() {
	this->stop = true;
	this->close = true;

	for (size_t i = 0; i < this->threads.size(); i++) {
		this->threads[i].join();
	}

	this->cameras.clear();
	this->threads.clear();
}