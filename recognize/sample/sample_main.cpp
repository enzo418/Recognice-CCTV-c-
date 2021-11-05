#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>
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

//    cv::FileStorage fileStorage(pathConfig, cv::FileStorage::READ);
//
//    int test_int = -1;
//    fileStorage["test_int"] >> test_int;
//    std::cout << "test_int: " << test_int << std::endl;
//
//    auto cfg = Observer::ConfigurationParser::ParseYAML(fileStorage);
//
//    std::cout << "mediaFolderPath: " << cfg.mediaFolderPath << std::endl;
//    std::cout << "scaleFactor: " << cfg.outputConfiguration.scaleFactor << std::endl;
//    std::cout << "api: " << cfg.telegramConfiguration.apiKey << std::endl;
//
//    cv::FileStorage fileStorageWrite(outputConfig, cv::FileStorage::WRITE);
//    Observer::ConfigurationParser::EmmitYAML(fileStorageWrite, cfg);
////    fileStorageWrite << "configuration" << "{" << "test" << "{" << "inside_test" << 2 << "}" << "test2" << "hola" << "}";

        YAML::Node config = YAML::LoadFile(outputConfig);
        auto cfg = Observer::ConfigurationParser::ParseYAML(config);

        // Convert to json (There is nothing wrong with it converting the
        // numbers to string since the client can parse them again into 
        // a number)
        YAML::Emitter emitter2;
        emitter2 << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << config;
        std::string out2(emitter2.c_str() + 1);  // Strip leading [ character
        std::cout << "Output with BeginSeq:\n" << out2 << '\n';


//    std::ofstream fout(outputConfig);
//    Observer::ConfigurationParser::EmmitYAML(fout, cfg);

//    Observer::ObserverCentral observer(cfg);
//    observer.Start();
}