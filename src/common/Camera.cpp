#include "Camera.hpp"

Camera::Camera(CameraConfiguration cameraConfig, ProgramConfiguration* programConfig, bool* stopFlag, cv::HOGDescriptor* hog) : config(cameraConfig), _programConfig(programConfig), _stop_flag(stopFlag), _descriptor(hog) {
	this->Connect();
}

void Camera::Connect(){
	this->capturer.open(this->config.url);
}

std::thread Camera::StartDetection() {
	return std::thread(&Camera::ReadFramesWithInterval, this);
}

void Camera::ReadFramesWithInterval() {
	#pragma region SetupVideoCapture

    // ==============
    //  consts/vars
    // ==============

    // if framesLeft > 0 then do a classifcation over the current frame
    ushort framesLeft = 0; 

    const ushort interval = this->_programConfig->msBetweenFrame;
    
    // higher interval -> lower max & lower interval -> higher max
    const ushort maxFramesLeft = (100.0 / (interval+0.0)) * 70; // 100 ms => max = 70 frames

    const int x = this->config.roi.point1.x;
    const int h = abs(this->config.roi.point2.y - this->config.roi.point1.y);
    const int w = abs(this->config.roi.point2.x - this->config.roi.point1.x);
    const ulong frameArea = w * h;

    const char* camName = &this->config.cameraName[0];
    const CAMERATYPE camType = this->config.type;
    const int changeThreshld = this->config.changeThreshold;

    const bool showPreview = this->_programConfig->showPreview;   
    const bool showProcessedImages = this->_programConfig->showProcessedFrames;
    const bool sendImageAfterChange = this->_programConfig->sendImageWhenDetectChange;

    const int maxRegisterPerPoint = 10;
        
    int totalNonZeroPixels = 0;
    
    // ammount of frame that recognition will be active before going to idle state    
    const uint8_t framesToRecognice = (100 / (interval + 0.0)) * 30; 

    // used to store the current frame to be able to compare it to the next one
    cv::Mat lastFrame;
    
    // used to store the diff frame
    cv::Mat diff;

    cv::VideoCapture capture(this->config.url);

    std::cout << "Opening " << camName << "..." << std::endl;
    assert(capture.isOpened());

    cv::Mat frame;
    cv::Mat invalid;
    cv::Mat frameToShow;

#pragma endregion

    auto timeLastframe = std::chrono::high_resolution_clock::now();
    bool newFrame = false;

    // ----------------
    //  Save&Upl image
    // ----------------
    auto timeLastSavedImage = std::chrono::high_resolution_clock::now();
    
    // -----------
    //  Messages
    // -----------
    auto lastMessageSended = std::chrono::high_resolution_clock::now();
    const int secondsBetweenMessages = this->_programConfig->secondsBetweenMessage;

    while (!*this->_stop_flag && capture.isOpened()) {
        const NISTATE camState = this->config.state;

        auto now = std::chrono::high_resolution_clock::now();
        auto intervalFrames = (now - timeLastframe) / std::chrono::milliseconds(1);
        if (intervalFrames >= interval) {
            capture.read(frame);
            timeLastframe = std::chrono::high_resolution_clock::now();
            newFrame = true;
        } else {            
            capture.read(invalid); // keep reading to avoid error on VC.
        }

        if (newFrame) {
#pragma region AnalizeFrame

            //assert(frame.rows != 0); // check if the frame is valid
            if (frame.rows == 0) {
                //std::cout << "Received a invalid frame from \"" << camName << "\". Restarting connection..." << std::endl;

                capture.release();
                capture.open(this->config.url);

                //framesCount--;
                assert(capture.isOpened());
                //std::cout << "Connected to \"" << camName << "\" successfully...." << std::endl;
                continue;
            }

            cv::resize(frame, frame, RESIZERESOLUTION);

            //if (showPreview)
                frameToShow = frame.clone();

            // Take the region of interes
            if (!this->config.roi.isEmpty()) {
                cv::Rect roi(this->config.roi.point1, this->config.roi.point2);
                frame = frame(roi);
            }

            // Then rotate it
            if (this->config.rotation != 0) ImageManipulation::RotateImage(frame, this->config.rotation);

            cv::cvtColor(frame, frame, cv::COLOR_RGB2GRAY);

            if (lastFrame.rows > 0) {
                cv::absdiff(lastFrame, frame, diff);

                // 45 works perfect with most of the cameras/resolution
                cv::threshold(diff, diff, this->config.noiseThreshold, 255, cv::THRESH_BINARY);

                totalNonZeroPixels = cv::countNonZero(diff);

                if (showProcessedImages && showPreview) {
                    // place diff image on top the frame img
                    cv::addWeighted(frame, 1, diff, 8, 12, diff);
					
                    this->frames.push_back(diff);
                }
            }

            if (totalNonZeroPixels > changeThreshld * 0.7) {
                std::cout << this->config.cameraName
                    << " Non zero pixels=" << totalNonZeroPixels
                    << " Configured threshold=" << changeThreshld
                    << std::endl;
            }
            lastFrame = frame;

            if (totalNonZeroPixels > changeThreshld) {
                if (camState != NI_STATE_DETECTED
                    && camType != CAMERA_SENTRY
                    && camState != NI_STATE_DETECTING)
                    this->config.state = NI_STATE_DETECTING; // Change the state of the camera
                                
                // every secondsBetweenMessages send a message to the telegram bot
                intervalFrames = (now - lastMessageSended) / std::chrono::seconds(1);
                if (intervalFrames >= secondsBetweenMessages) {
                    // Play a sound
                    this->pendingAlerts.push_back(Message());

                    if(sendImageAfterChange){                        
                        // and then send a message
                        const char* txt = "temp_image";
                        Message msg = Message(frameToShow, txt);                        
                        this->pendingAlerts.push_back(msg);
                    }else{
                        // and then send a message
                        char msg[100];
                        snprintf(msg, MESSAGE_SIZE, "[W] Motion detected in %s", this->config.cameraName.c_str());
                        this->pendingAlerts.push_back(Message(msg));
                    }

                    lastMessageSended = std::chrono::high_resolution_clock::now();
                }

                // Increment frames left
                if (framesLeft < maxFramesLeft)
                    framesLeft += framesToRecognice;
            }

            if (camType == CAMERA_SENTRY) {
                if (framesLeft > 0) {
                    if (camState != NI_STATE_DETECTING) {
                        this->config.state = NI_STATE_DETECTING;
                    }
                    framesLeft--;
                } else {
                    this->config.state = NI_STATE_SENTRY;
                }

                newFrame = false;
                if (showPreview && !showProcessedImages)
                    this->frames.push_back(frameToShow);
            } else {
                std::vector<cv::Rect> detections;
                std::vector< double > foundWeights;
                if (framesLeft > 0) {
                    this->_descriptor->detectMultiScale(frame, detections, foundWeights, this->config.hitThreshold, cv::Size(8, 8), cv::Size(4, 4), 1.05);
                    size_t detectSz = detections.size();

                    if(detectSz > 0){
                        int areaMatchEntry = 0;
                        int areaMatchExit = 0;

                        for (size_t i = 0; i < detectSz; i++) {
                            detections[i].x += x;
                            cv::Scalar color = cv::Scalar(0, foundWeights[i] * foundWeights[i] * 200, 0);
                            cv::rectangle(frameToShow, detections[i], color);
                            
                            putText(frame, std::to_string(foundWeights[i]), Utils::BottomRightRectangle(detections[i]), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));

                            areaMatchEntry += Utils::OverlappingArea(this->config.areasDelimiters.rectEntry, detections[i]);
                            areaMatchExit += Utils::OverlappingArea(this->config.areasDelimiters.rectExit, detections[i]);
                        }
                        
                        RegisterPoint point = areaMatchEntry > areaMatchExit ? RegisterPoint::entryPoint : RegisterPoint::exitPoint;
                        bool shouldSave = false;
                        int count = 0;

                        size_t regSz = this->registers.size();
                        for (size_t i = 0; i < regSz; i++)
                            if(this->registers[i].firstPoint == point) 
                                count++;                      

                        if(count < maxRegisterPerPoint){
                            // Save a register 
                            Register regp;
                            regp.id = this->registers.size();
                            time_t ntime = time(0);
                            regp.time_point = localtime(&ntime);
                            regp.finished = false;
                            regp.firstPoint = point;
                            
                            std::cout << "[" << this->config.order << "]" << "Pushed a register " << (regp.firstPoint == RegisterPoint::entryPoint ? "entry" : "exit") 
                            << " time = " << regp.time_point->tm_min << "m:" << regp.time_point->tm_sec << "s"
                            << " areaEntry = " <<  areaMatchEntry << " areaExit =" << areaMatchExit
                            << std::endl;

                            this->registers.push_back(regp);
                        }else{
                            std::cout << "[" << this->config.order << "]" << "Canno't push max reached, count = " << count << " size= " << regSz << std::endl;
                        }

                        // Send a telegram message
                        now = std::chrono::high_resolution_clock::now();
                        auto time = (now - timeLastSavedImage) / std::chrono::seconds(1);
                        if (time >= this->_programConfig->secondsBetweenImage) {
                            this->config.state = NI_STATE_DETECTED;
                            Message msg = Message(frameToShow, "");
                            snprintf(msg.text, MESSAGE_SIZE, "%s", Utils::GetTimeFormated().c_str());
                            this->pendingAlerts.push_back(msg);
                            std::cout << "[" << this->config.order << "]" << "Pushed image seconds=" << time << " of " << this->_programConfig->secondsBetweenImage << std::endl;
                            timeLastSavedImage = std::chrono::high_resolution_clock::now();
                        }
                    }

                    framesLeft--;
                    //std::cout << camName << " -- Frames " << framesLeft << std::endl;
                }
#pragma endregion

                if (framesLeft == 0) {
                    this->config.state = NI_STATE_SENTRY;
                }

                newFrame = false;

                if (showPreview && !showProcessedImages)
                    this->frames.push_back(std::move(frameToShow));
            }
        }
    }

    std::cout << "Closed connection with " << camName << std::endl;

    capture.release();
}