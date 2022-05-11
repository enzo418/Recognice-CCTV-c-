#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <ratio>
#include <string>
#include <thread>

#include "../../Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../../Observer/src/Domain/ObserverCentral.hpp"

void Cameras(Observer::Configuration* cfg, bool useCompression);

using namespace Observer;

struct CameraThread {
    std::thread thread;
    std::shared_ptr<CameraObserver> camera;
};

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file}"
        "{seconds |  30   | seconds}"
        "{use_compression | off | test to compress the image}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    bool useCompression = false;
    if (parser.has("use_compression")) {
        useCompression =
            parser.get<std::string>("use_compression", true) != "off";
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    Cameras(&cfg, useCompression);
}

void Cameras(Observer::Configuration* cfg, bool useCompression) {
    BufferedSource source;
    OBSERVER_INFO("Try open");

    source.TryOpen(cfg->camerasConfiguration[0].url);

    OBSERVER_INFO("Check ok");
    if (!source.IsOk()) {
        OBSERVER_ERROR("Couldn't open the camera");
    }

    OBSERVER_INFO("Get");
    Frame frame;
    std::vector<unsigned char> buffer;

    Timer<std::chrono::nanoseconds> timer;

    while (source.IsOk()) {
        if (source.IsFrameAvailable()) {
            timer.Start();

            frame = source.GetFrame();

            auto timeGetFrame = timer.GetDurationAndRestart();

            if (useCompression) {
                frame.EncodeImage(".jpg", 90, buffer);
                OBSERVER_TRACE("Compressed!");
            }

            auto timeCompressFrame = timer.GetDuration();

            OBSERVER_TRACE(
                "Frame width: {} | Get took: {} ns | Compress Took: {} ns | "
                "total: {} ns",
                frame.GetSize().width, timeGetFrame, timeCompressFrame,
                timeGetFrame + timeCompressFrame);
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
