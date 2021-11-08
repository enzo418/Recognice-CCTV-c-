#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <thread>
#include "../src/ConfigurationParser.hpp"
#include "../src/ObserverCentral.hpp"

int main(int argc, char** argv) {
    std::string pathConfig;
    std::string outputConfig = "./config_ouput.yml";

    const std::string keys =
            "{help h       |            | show help message}"
            "{config_path  | ./config.yml | path of the configuration file}";

    cv::CommandLineParser parser (argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    std::cout << "file: " << pathConfig << " curr dir: " << std::filesystem::current_path().string() << std::endl;

//    auto cfg = Observer::ConfigurationParser::ParseYAML(fileStorage);
//
//    std::cout << "mediaFolderPath: " << cfg.mediaFolderPath << std::endl;
//    std::cout << "scaleFactor: " << cfg.outputConfiguration.scaleFactor << std::endl;
//    std::cout << "api: " << cfg.telegramConfiguration.apiKey << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    // Convert to json (There is nothing wrong with it converting the
    // numbers to string since the client can parse them again into 
    // a number)
    Observer::ConfigurationParser::EmmitJSON("output.json", cfg);
    
    Observer::ConfigurationParser::EmmitYAML(outputConfig, cfg);

    Observer::ObserverCentral observer(cfg);
    observer.Start();

    std::this_thread::sleep_for(std::chrono::hours(30));
}