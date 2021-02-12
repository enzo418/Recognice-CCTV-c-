#include "recognize.hpp"

Recognize::Recognize() {
	this->notificationWithMedia = std::make_unique<moodycamel::ReaderWriterQueue<std::pair<Notification::Type, std::string>>>(100);
}

void Recognize::Start(const Configurations& configs, bool startPreviewThread, bool startActionsThread) {	
	this->close = false;
	this->stop = false;
	
	this->programConfig = configs.programConfig;
	this->camerasConfigs = configs.camerasConfigs;

	bool usesObjectDetection = false;
	// Remove disabled cameras
	size_t sz = this->camerasConfigs.size();
	for (size_t i = 0; i < sz; i++) {
		if (this->camerasConfigs[i].type == CAMERA_DISABLED || this->camerasConfigs[i].url.empty()) {
			this->camerasConfigs.erase(this->camerasConfigs.begin() + i);
			i--;
			sz--;
		} else if (this->camerasConfigs[i].type == CAMERA_ACTIVE) {
			usesObjectDetection = true;
		}
	}
	
	if (this->camerasConfigs.size() == 0) {
		/// TODO: throw custom exception
		std::cout << "Cameras size 0. Exiting." << std::endl;
		std::exit(-1);
	}
	
	if (usesObjectDetection) {
		if (this->programConfig.detectionMethod == DetectionMethod::HogDescriptor && lastDetectionMethod != DetectionMethod::HogDescriptor) {
			if (lastDetectionMethod == DetectionMethod::YoloDNN_V4)
				delete this->net;
			
			this->hogDescriptor = new cv::HOGDescriptor();
			this->hogDescriptor->setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
			
			lastDetectionMethod = DetectionMethod::HogDescriptor;
		} else if (this->programConfig.detectionMethod == DetectionMethod::YoloDNN_V4 && lastDetectionMethod != DetectionMethod::YoloDNN_V4) {
			#ifdef WITH_CUDA
			if (lastDetectionMethod == DetectionMethod::HogDescriptor)
				delete this->hogDescriptor;
				
			lastDetectionMethod = DetectionMethod::YoloDNN_V4;
			
			this->net = new cv::dnn::Net();
			*this->net = cv::dnn::readNetFromDarknet("yolov4.cfg", "yolov4.weights");
			this->net->setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
			this->net->setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
			output_names = this->net->getUnconnectedOutLayersNames();
			
			{
				std::ifstream class_file("classes.txt");
				if (!class_file)
				{
					std::cerr << "failed to open classes.txt\n";
					std::exit(-1);
				}

				std::string line;
				while (std::getline(class_file, line)) {
					if (!line.empty()) {
						class_names.push_back(line);
						this->num_classes++;
					}
				}
			}
			#endif // WITH_CUDA
		}
	}
	
	this->indexMainThreadCameras = this->threads.size();
	this->StartCamerasThreads();

	if (startPreviewThread) {
		std::cout << "pushed thread of preview in " << this->threads.size() << std::endl;
		// Start the thread to show the images captured.
		this->threads.push_back(std::thread(&Recognize::StartPreviewCameras, this));
	}

//	if (startActionsThread) {
//		std::cout << "pushed thread of actions bot in " << this->threads.size() << std::endl;
//		this->threads.push_back(std::thread(&Recognize::StartActionsBot, this));
//	}

	if (programConfig.telegramConfig.useTelegramBot || programConfig.localNotificationsConfig.useLocalNotifications) {
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
		this->cameras.push_back(std::make_unique<Camera>(config, &programConfig, this->hogDescriptor));
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

	cv::Mat dequeRes;
	while (!stop && programConfig.showPreview) {
		if (this->cameras.size() == amountCameras) {
			// if all cameras are in sentry state
			bool allCamerasInSentry = true;

			for (size_t i = 0; i < amountCameras; i++) {
				// try_dequeue Returns false if the queue was empty
				if (cameras[i]->frames->try_dequeue(dequeRes)) {
					// if the vector pos i has no frame
					if (!ready[cameras[i]->config->order]) {
						// take the first frame and delete it
						frames[cameras[i]->config->order] = dequeRes.clone();

						if (showAreaCameraSees && !showProcessedFrames) {
							cv::Scalar color = cv::Scalar(255, 0, 0);
							cv::rectangle(frames[cameras[i]->config->order], cameras[i]->config->roi, color);
						}

						// cameras[i]->frames.erase(cameras[i]->frames.begin());
						
						ready[cameras[i]->config->order] = true;
						
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

	std::cout << "Preview thread closed" << std::endl;

	// cv::destroyWindow("Preview Cameras");
}

void Recognize::StartNotificationsSender() {
	cv::Mat frame;
	std::string date;

	while (!stop) {
		for (auto &&camera : cameras) {

			if (programConfig.useGifInsteadImage) {
				size_t sz = camera->gifsReady.size();
				for (size_t i = 0; i < sz; i++) {
					bool eraseGifs = false;

					GifFrames* gif = camera->gifsReady[i].release();
					// Send gif
					if (gif->getState() == State::Ready) {
						// -----------------------------------------------------------
						// Take before and after frames and combine them into a .gif
						// -----------------------------------------------------------
						const std::string identifier = std::to_string(clock());
						const std::string imageFolder = "./" + programConfig.imagesFolder + "/" ;
						const std::string imagesIdentifier = imageFolder + std::to_string(camera->config->order);
						const std::string root = imageFolder + identifier;
						const std::string gifPath = root + ".gif";
						std::string location;
						const size_t gframes = programConfig.numberGifFrames.framesAfter + programConfig.numberGifFrames.framesBefore;

						if (gif->isValid()) {
							auto now = std::chrono::high_resolution_clock::now();
							auto intervalFrames = (now - camera->lastImageSended) / std::chrono::seconds(1);
							if (intervalFrames >= this->programConfig.secondsBetweenImage) {
								eraseGifs = true;

								std::vector<cv::Mat> frames = gif->getFrames();

								if (this->programConfig.telegramConfig.useTelegramBot
									|| this->programConfig.localNotificationsConfig.useLocalNotifications) {
									camera->pendingNotifications.push_back(Notification::Notification("Movimiento detectado en la camara " + camera->config->cameraName));
								}
								
								// if (frames.avrgDistanceFrames > 70) {
								for (size_t i = 0; i < frames.size(); i++) {
									location = imagesIdentifier + "_" + std::to_string((int)i) + ".jpg";

									cv::imwrite(location, frames[i]);
								}

								std::string command = "convert -resize " + std::to_string(programConfig.gifResizePercentage) + "% -delay 23 -loop 0 " + imagesIdentifier + "_{0.." + std::to_string(gframes-1) + "}.jpg " + gifPath;

								std::system(command.c_str());

								if (this->programConfig.telegramConfig.useTelegramBot)
									TelegramBot::SendMediaToChat(gifPath, "Movimiento detectado. " + gif->getText(), programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey, true);

								camera->lastImageSended = std::chrono::high_resolution_clock::now();

								this->notificationWithMedia->try_emplace(
									std::pair<Notification::Type, std::string>(Notification::IMAGE, gifPath));
							}
						}
					}

					delete gif;

					// if a gif was sent then delete all the others 
					if (eraseGifs) {
						camera->gifsReady.clear();
						sz = 0;
					} else {
						camera->gifsReady.erase(camera->gifsReady.begin() + i);
						sz--;
						i--;
					}
				}
			}

			size_t size = camera->pendingNotifications.size();
			for (size_t i = 0; i < size; i++) {	
				std::cout << "Sending notification of type " << camera->pendingNotifications[i].type << std::endl;

				std::string data = camera->pendingNotifications[i].send(programConfig);
				if (camera->pendingNotifications[i].type == Notification::IMAGE 
						|| camera->pendingNotifications[i].type == Notification::SOUND
						|| (camera->pendingNotifications[i].type == Notification::TEXT 
								&& this->programConfig.localNotificationsConfig.sendTextWhenDetectChange)) {
					this->notificationWithMedia->try_emplace(
							std::pair<Notification::Type, std::string>(
								camera->pendingNotifications[i].type, 
								data
								)
						);
				}
			}

			// This proc shouldn't clear all the notifcations since it's a multithread process :p
			camera->pendingNotifications.clear();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	
	std::cout << "Notifiaction thread closed" << std::endl;
}

void Recognize::CloseAndJoin() {
	this->stop = true;
	this->close = true;

	for (size_t i = 0; i < this->threads.size(); i++) {
		this->threads[i].join();
	}
	
	std::cout << "Threads from main closed" << std::endl;
	
	size_t sz = this->cameras.size();
	for(size_t i = 0; i < sz; i++) {
		Camera* camera = this->cameras[i].release();

		if (camera->cameraThread->joinable()) {
			camera->close = true;
			camera->cameraThread->join();
		}
		
		const std::string n = camera->config->cameraName;
		std::cout << "Release " << n << std::endl;
		delete camera;
		std::cout << "Released " << n << std::endl;
		
		std::cout << "Size: " << this->cameras.size() << std::endl;
	}
	
	std::cout << "Threads from cameras closed" << std::endl;

	this->cameras.clear();
	
	std::cout << "Cameras vector cleaned" << std::endl;
	
	this->threads.clear();
	
	std::cout << "Threads vector cleared" << std::endl;
}

std::vector<std::tuple<cv::Rect, double, std::string>> Recognize::Detect(cv::Mat& frame, CameraConfiguration& cfg) {
	std::vector<std::tuple<cv::Rect, double, std::string>> results;	
	
	if (this->programConfig.detectionMethod == DetectionMethod::HogDescriptor) {
		std::vector<cv::Rect> detections;
		std::vector<double> foundWeights;
		
		this->hogDescriptor->detectMultiScale(frame, detections, foundWeights, cfg.hitThreshold, cv::Size(8, 8), cv::Size(4, 4), 1.05);
		
		for (size_t i = 0; i < detections.size(); i++) {
			results.push_back(std::make_tuple(detections[i], foundWeights[i], ""));
		}
	} else if (this->programConfig.detectionMethod == DetectionMethod::YoloDNN_V4) {
		#ifdef WITH_CUDA
		cv::Mat blob;
		std::vector<cv::Mat> detectionsFrames;
		cv::dnn::blobFromImage(frame, blob, 0.006921, cv::Size(608,608), cv::Scalar(), true, false, CV_32F);
		this->net->setInput(blob);
		this->net->forward(detectionsFrames, this->output_names);
		
		std::vector<int> indices[num_classes];
		std::vector<cv::Rect> boxes[num_classes];
		std::vector<float> scores[num_classes];

		for (auto& output : detectionsFrames) {
			const auto num_boxes = output.rows;
			for (int i = 0; i < num_boxes; i++) {
				auto x = output.at<float>(i, 0) * frame.cols;
				auto y = output.at<float>(i, 1) * frame.rows;
				auto width = output.at<float>(i, 2) * frame.cols;
				auto height = output.at<float>(i, 3) * frame.rows;
				cv::Rect rect(x - width/2, y - height/2, width, height);

				for (int c = 0; c < num_classes; c++) {
					auto confidence = *output.ptr<float>(i, 5 + c);
					if (confidence >= CONFIDENCE_THRESHOLD) {
						boxes[c].push_back(rect);
						scores[c].push_back(confidence);
					}
				}
			}
		}
					
		for (int c= 0; c < num_classes; c++) {
			cv::dnn::NMSBoxes(boxes[c], scores[c], 0.0, NMS_THRESHOLD, indices[c]);
			
			for (size_t i = 0; i < indices[c].size(); ++i) {
				int idx = indices[c][i];
				
				results.push_back(std::make_tuple(boxes[c][idx], scores[c][idx], class_names[c]));
			}
		}
		#endif // WITH_CUDA
	}
	
	return results;
}