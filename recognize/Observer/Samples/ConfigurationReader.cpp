#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/Configuration/NLHJSONConfiguration.hpp"
#include "observer/Domain/ObserverCentral.hpp"

using namespace Observer;

int main(int argc, char** argv) {
    Observer::LogManager::Initialize();

    const std::string keys =
        "{help h            |               | show help message}"
        "{config_path       | ./config.json | path of the configuration file}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }
    LocalWebNotificationsConfiguration localWebConfiguration = {
        .webServerUrl = "localhost:9000/test"};

    localWebConfiguration.enabled = true;
    localWebConfiguration.drawTraceOfChangeOn = ETrazable::NONE;
    localWebConfiguration.secondsBetweenImageNotification = 5.1;
    localWebConfiguration.secondsBetweenTextNotification = 5.1;
    localWebConfiguration.secondsBetweenVideoNotification = 5.1;
    localWebConfiguration.notificationsToSend = ENotificationType::VIDEO |
                                                ENotificationType::TEXT |
                                                ENotificationType::IMAGE;

    json camConfig = localWebConfiguration;

    std::cout << camConfig.dump(2) << std::endl;

    // std::cout << "file: " << pathConfig
    //           << " curr dir: " << std::filesystem::current_path().string()
    //           << std::endl;

    // auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);
}
