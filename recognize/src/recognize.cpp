#include "recognize.hpp"

Recognize::Recognize() {
	this->notificationWithMedia = std::make_unique<moodycamel::ReaderWriterQueue<std::tuple<Notification::Type, std::string, ulong>>>(100);
}

bool Recognize::Start(const Configurations& configs, bool startPreviewThread, bool startActionsThread) {	
	bool success = true;

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
		return false;
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

	return success;
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

	const bool sendGif = this->programConfig.telegramConfig.sendGifWhenDetectChange
						|| this->programConfig.localNotificationsConfig.sendGifWhenDetectChange;

	const bool useNotifications = this->programConfig.telegramConfig.useTelegramBot
									|| this->programConfig.localNotificationsConfig.useLocalNotifications;

	std::filesystem::path cwd = std::filesystem::current_path().string() + "/";

	// Order to send the notifications based on type notification
	Notification::Type notificationTypesPriority[] = {
			Notification::TEXT, 
			Notification::SOUND, 
			Notification::IMAGE, 
			Notification::VIDEO, 
			Notification::GIF
	};

	while (!stop && useNotifications) {
		for (auto &&camera : cameras) {

			if (sendGif || programConfig.analizeBeforeAfterChangeFrames) {
				size_t sz = camera->gifsReady.size();
				for (size_t i = 0; i < sz; i++) {
					bool eraseGifs = false;

					auto now = std::chrono::high_resolution_clock::now();
					auto intervalFrames = (now - camera->lastImageSended) / std::chrono::seconds(1);

					GifFrames* gif = camera->gifsReady[i].release();
					// Ready and didn't send a gif since secondsBetweenImage and is valid
					if (gif->getState() == State::Ready 
							&& intervalFrames >= this->programConfig.secondsBetweenImage
							&& gif->isValid()) {
						this->currentGroupID += 1;

						const bool saveChangeVideo = this->programConfig.saveChangeInVideo;
						const std::string imageFolder = "./" + programConfig.imagesFolder + "/" ;
						const ulong& group_id = this->currentGroupID;
						const std::string identifier = std::to_string(group_id);

						std::string videoPath = imageFolder + std::to_string(camera->config->order) + "_" + identifier + ".mp4";

						if (saveChangeVideo) {
							// -------------------------------------------
							//  Concat the two camera videos into a video
							// -------------------------------------------

							std::filesystem::path file1 = (cwd / std::filesystem::path(camera->videosPath[!camera->currentIndexVideoPath])).lexically_normal();
							std::filesystem::path file2 = (cwd / std::filesystem::path(camera->videosPath[camera->currentIndexVideoPath])).lexically_normal();

							camera->videosPath[camera->currentIndexVideoPath] = "";

							// get file2 video length before starting the new video
							auto file2VideoLength = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - camera->lastVideoStartTime).count();

							// Release current video and and unlock overwrite
							camera->ReleaseAndOpenChangeVideo(false);

							camera->videoLocked = false;

							std::cout << "\n[V] Length of the current video: " << file2VideoLength << "\n";

							// if the older video exist
							if (file1.has_filename() && file2VideoLength >= 2) {
								std::cout << "\n[V] Video has 2 files.\n";

								// command to concat the files from list.txt
								std::string ffmpegCommand = "ffmpeg -hide_banner -loglevel error -y -f concat -safe 0 -i list.txt -c copy " + videoPath;

								// command to create the list of videos to concat
								std::string listCommand("echo \"file '" + file1.string() + "'\nfile '" + file2.string() + "'\" > list.txt");

								std::cout << "[F] LIST-COMMAND: " << listCommand << "\n\n"
											<< "    FFMPEG-COMMAND: " << ffmpegCommand << std::endl << std::endl;

								// create list
								std::system(listCommand.c_str());

								// concat videos
								std::system(ffmpegCommand.c_str());

								camera->lastSendedVideoPath = videoPath;
							} else {
								// else if there is only 1 video check if the length is > 5, if not send the last one else send the newest
								std::cout << "\n[V] Video has 1 file" << std::endl 
											<< "\n[V] Last video path: " << camera->lastSendedVideoPath << std::endl
											<< "\n[V] File2: " << file2.string() << std::endl
											<< std::endl;
								if (file2VideoLength <= 5 && camera->lastSendedVideoPath.length() > 0) {
									videoPath = camera->lastSendedVideoPath;
								} else {
									videoPath = file2.string();
								}
							}

							camera->pendingNotifications.push_back(Notification::Notification(videoPath, camera->config->cameraName, group_id));

							// This works because the max length of the video is < than time between image notification 
							// else this brokes
							camera->sendChangeVideoContinuation = true;
							camera->continuation_group_id = group_id;
						} else {
							// clear it so we don't show anything on the webpage
							videoPath = "";
						}

						// -----------------------------------------------------------
						//  Take before and after frames and combine them into a .gif
						// -----------------------------------------------------------
						const std::string imagesIdentifier = imageFolder + std::to_string(camera->config->order);
						const std::string root = imageFolder + identifier;
						const std::string gifPath = saveChangeVideo ? videoPath.substr(0, videoPath.length() - 4) + ".gif" : root + ".gif";
						
						std::string location;
						const size_t gframes = programConfig.numberGifFrames.framesAfter + programConfig.numberGifFrames.framesBefore;

						eraseGifs = true;

						std::vector<cv::Mat> frames = gif->getGifFrames();

						if (this->programConfig.analizeBeforeAfterChangeFrames) {// text notification
							std::string message = Utils::FormatNotificationTextString(this->programConfig.messageOnTextNotification, camera->config->cameraName);
							camera->pendingNotifications.push_back(Notification::Notification(message, group_id));
						
							// image notification
							cv::Mat detected_frame = frames[gif->indexFirstFrameWithChangeDetected()];

							std::vector<cv::Point> trace = gif->getFindingTrace();
							for (size_t i = 0; i < trace.size(); i++) {
								trace[i].x += camera->config->roi.x;
								trace[i].y += camera->config->roi.y;

								cv::circle(detected_frame, trace[i], 5, cv::Scalar(0, 0, 255), -1);
								
								if (i + 1 < trace.size()) {
									cv::line(detected_frame, trace[i], trace[i+1], cv::Scalar(0,255,0));
								}
							}
							
							camera->pendingNotifications.push_back(Notification::Notification(detected_frame, message, true, group_id));
						}

						if (sendGif) {
							// This for sentence is "quite" fast so we can do it here and notifications will not be delayed
							for (size_t i = 0; i < frames.size(); i++) {
								location = imagesIdentifier + "_" + std::to_string((int)i) + ".jpg";

								cv::imwrite(location, frames[i]);

								// this if introduces from 20% to 50% extra of cpu time
								// doesn't really matter too much since we are talking of
								// about 50 ~ 200 ms more in my tests with 20 frames.
								// if (saveChangeVideo)
								// 	camera->AppendFrameToVideo(frames[i]);
							}
							
							// if (saveChangeVideo)
							// 	camera->ReleaseChangeVideo();

							std::string command = "convert -resize " + std::to_string(programConfig.gifResizePercentage) + "% -delay 23 -loop 0 " + imagesIdentifier + "_{0.." + std::to_string(gframes-1) + "}.jpg " + gifPath;

							camera->pendingNotifications.push_back(Notification::Notification(gifPath, gif->getText(), command, group_id));
						} 
						
						camera->lastImageSended = std::chrono::high_resolution_clock::now();
					} else {
						camera->videoLocked = false;
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
			
			// iterate types priority
			for (auto&& type : notificationTypesPriority) {				
				// iterate notifications vector
				for (size_t i = 0; i < size; i++) {
					Notification::Notification& notf = camera->pendingNotifications[i];
					
					if (notf.type == type && !notf.sended) {
						std::cout << "Sending notification of type " << camera->pendingNotifications[i].type << std::endl;
						
						// Since merging multiple images into one file is cpu intensive, 
						// and can take up to a few seconds on different processors, 
						// the media types (gif/videos) need to be converted as they are sent, 
						// so that they don't delay the rest of the notifications in the queue. 
						// A different solution is to build it in another thread, 
						// but it requires coordination. And I think it is an overkill.
						if (notf.type == Notification::GIF) {				
							notf.buildMedia();
						}

						// send to telegram
						std::string data = notf.send(programConfig);
						
						// send to local
						if (programConfig.localNotificationsConfig.useLocalNotifications
							&& notf.type == Notification::SOUND
							|| (
									notf.type == Notification::IMAGE
									&& this->programConfig.localNotificationsConfig.sendImageWhenDetectChange
								)
							|| (
									// check if user wants text messages in local notifications
									notf.type == Notification::TEXT 
									&& this->programConfig.localNotificationsConfig.sendTextWhenDetectChange
								)					
							|| (
									notf.type == Notification::GIF
									&& this->programConfig.localNotificationsConfig.sendGifWhenDetectChange
							)
							|| (
								notf.type == Notification::VIDEO
								/**&& send video as notification? */
							)
						)
						{
							this->notificationWithMedia->try_emplace(
									std::tuple<Notification::Type, std::string, ulong>(
										notf.type, 
										data,
										notf.getGroupId()
										)
								);
						}

						notf.sended = true;
					}
				}
			}

			camera->pendingNotifications.erase(camera->pendingNotifications.begin(), camera->pendingNotifications.begin() + size);
			// // This proc shouldn't clear all the notifcations since it's a multithread process :p
			// camera->pendingNotifications.clear();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	
	std::cout << "Notification thread closed" << std::endl;
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