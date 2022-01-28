#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "../../Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../../Observer/src/Domain/ObserverCentral.hpp"

void Cameras(Observer::Configuration* cfg);

using namespace Observer;

struct CameraThread {
    std::thread thread;
    std::shared_ptr<CameraObserver> camera;
};

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file}"
        "{seconds |  30   | seconds}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
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

    Cameras(&cfg);
}

void Cameras(Observer::Configuration* cfg) {
    BufferedSource source;
    OBSERVER_INFO("Try open");

    source.TryOpen(cfg->camerasConfiguration[0].url);

    OBSERVER_INFO("Check ok");
    if (!source.IsOk()) {
        OBSERVER_ERROR("Couldn't open the camera");
    }

    OBSERVER_INFO("Get");
    Frame frame;
    while (source.IsOk()) {
        if (source.IsFrameAvailable()) {
            frame = source.GetFrame();
            OBSERVER_TRACE("Frame width: {}", frame.GetSize().width);
        }
    }
    /*
        std::vector<CameraThread> cams;
        cams.reserve(cfg->camerasConfiguration.size());

        for (auto& cfg : cfg->camerasConfiguration) {
            if (cfg.type != ECameraType::DISABLED) {
                CameraThread ct;
                ct.camera = std::make_shared<CameraObserver<FrameType>>(&cfg);
                ct.thread =
                    std::thread(&CameraObserver<FrameType>::Start, ct.camera);

                cams.push_back(std::move(ct));
            }
        }

        for (auto& thread : cams) {
            if (thread.thread.joinable()) {
                thread.thread.join();
            }
        }*/
}
