#include <chrono>
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>

#include "../src/ConfigurationParser.hpp"
#include "../src/ObserverCentral.hpp"
#include "../src/implementations/opencv/ImageDisplay.hpp"
#include "../src/implementations/opencv/ImagePersistence.hpp"
#include "../src/implementations/opencv/ImageTransformation.hpp"
#include "../src/implementations/opencv/VideoSource.hpp"
#include "../src/implementations/opencv/VideoWriter.hpp"
#include "../src/log/log.hpp"

int main(int argc, char** argv) {
    std::string pathConfig;
    std::string outputConfig = "./config_ouput.yml";

    const std::string keys =
        "{help h       |            | show help message}"
        "{config_path  | ./config.yml | path of the configuration file}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    Observer::ObserverCentral<cv::Mat> observer(cfg);
    observer.Start();

    std::this_thread::sleep_for(std::chrono::hours(30));
}