#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"

void OpenCamera(Observer::Configuration* cfg, int minutes);

using namespace Observer;

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.json | path of the configuration file}"
        "{camera_index      | 0 | index of the camera to open}";
    ;

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    int cameraIndex = 0;
    if (parser.has("camera_index")) {
        cameraIndex = parser.get<int>("camera_index");
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg =
        Observer::ConfigurationParser::ConfigurationFromJsonFile(pathConfig);

    OpenCamera(&cfg, cameraIndex);
}

void OpenCamera(Observer::Configuration* cfg, int cameraIndex) {
    auto camCfg = &cfg->cameras[cameraIndex];
    Observer::VideoSource source;

    source.Open(camCfg->url);

    if (!source.isOpened()) {
        OBSERVER_ERROR("Could not open the camera.");
        return;
    }

    Observer::ImageDisplay::Get().CreateWindow("Image");
    Observer::ImageDisplay::Get().CreateWindow("mask");

    Frame imageMask;

    imageMask = Frame(source.GetSize(), 1);

    std::cout << "channels: " << imageMask.GetNumberChannels() << std::endl;

    std::cout << "masks: " << camCfg->processingConfiguration.masks.size()
              << std::endl;

    for (auto& mask : camCfg->processingConfiguration.masks) {
        ImageDraw::Get().FillConvexPoly(imageMask, mask, ScalarVector::White());
    }

    std::cout << "channels after: " << imageMask.GetNumberChannels()
              << std::endl;

    imageMask.Resize(Size(640, 360));

    Observer::ImageDisplay::Get().ShowImage("mask", imageMask);

    Frame frame;
    int i = 0;
    while (true) {
        source.GetNextFrame(frame);

        frame.Resize(Size(640, 360));

        frame.Mask(imageMask);

        Observer::ImageDisplay::Get().ShowImage("Image", frame);
    }

    source.Close();
}
