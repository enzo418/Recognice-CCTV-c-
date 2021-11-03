#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include "../src/ConfigurationParser.hpp"

int main(int argc, char** argv) {
    std::string pathConfig = "./config.yml";

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

    cv::FileStorage fileStorage(pathConfig, cv::FileStorage::READ);

    int test_int = -1;
    fileStorage["test_int"] >> test_int;
    std::cout << "test_int: " << test_int << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(fileStorage);

    std::cout << "mediaFolderPath: " << cfg.mediaFolderPath << std::endl;
    std::cout << "scaleFactor: " << cfg.outputConfiguration.scaleFactor << std::endl;
    std::cout << "api: " << cfg.telegramConfiguration.apiKey << std::endl;
}