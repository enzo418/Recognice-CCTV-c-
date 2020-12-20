#include "../src/recognize.hpp"

// For another way of detection see https://sites.google.com/site/wujx2001/home/c4 https://github.com/sturkmen72/C4-Real-time-pedestrian-detection/blob/master/c4-pedestrian-detector.cpp
int main(int argc, char* argv[]){
	bool isUrl = false;
	std::string pathConfig = "./config.ini";    

	const std::string keys = 
		"{help h       |            | show help message}"
		"{url          |            | url of the camera to obtain a image}"
		"{config_path  | config.ini | path of the configuration file}";

	cv::CommandLineParser parser (argc, argv, keys);

	if (parser.has("help")) {
		parser.printMessage();
		return 0;
	}

	if (parser.has("url")) {
		std::hash<std::string> hasher;
		std::string url = parser.get<std::string>("url");
		size_t name = hasher(url);

		std::cout << "URL => " << url;

		return 0;
		
		char path[75];
		sprintf_s(path, "%lu.jpg", name);

		if (!Utils::FileExist(path)) {
			cv::VideoCapture vc(url);

			cv::Mat frame;

			if (vc.isOpened()) {
				vc.read(frame);
				vc.release();

				cv::resize(frame, frame, RESIZERESOLUTION);

				cv::imwrite(path, frame);                    
			} else {
				return -1;
			}
		}

		std::cout << "image_name=" << path << std::endl;

		return 0;
	}

	if (parser.has("config_path")) {
		pathConfig = parser.get<std::string>("config_path");
	}

	// Get the cameras and program configurations
	Configurations cfgs = ConfigurationFile::ReadConfigurations(pathConfig);

	assert(cfgs.camerasConfigs.size() != 0);

	Recognize recognize;

	recognize.Start(std::ref(cfgs), cfgs.programConfig.showPreview, cfgs.programConfig.telegramConfig.useTelegramBot);

	// signal(SIGINT, signal_callback_handler);

	std::cout << "Press a key to stop the program.\n";
	std::getchar();

	recognize.close = true;
	recognize.stop = true;

	for (size_t i = 0; i < recognize.threads.size(); i++) {
		std::cout << "joining thread " << i << std::endl;
		recognize.threads[i].join();
		std::cout << "joined thread " << i << std::endl; 
	}	
}