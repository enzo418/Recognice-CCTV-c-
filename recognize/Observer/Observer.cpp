#include <chrono>
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>

#include "../Observer/Implementations/opencv/ImageDisplay.hpp"
#include "../Observer/Implementations/opencv/ImagePersistence.hpp"
#include "../Observer/Implementations/opencv/ImageTransformation.hpp"
#include "../Observer/Implementations/opencv/VideoSource.hpp"
#include "../Observer/Implementations/opencv/VideoWriter.hpp"
#include "../Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../Observer/src/Domain/ObserverCentral.hpp"
#include "../Observer/src/Log/log.hpp"
#include "src/Domain/Configuration/Configuration.hpp"
#include "src/Domain/VideoSource.hpp"
#include "src/Domain/VideoWriter.hpp"
#include "src/ImageDisplay.hpp"
#include "src/ImageTransformation.hpp"
#include "src/Timer.hpp"
#include "src/Utils/SpecialFunctions.hpp"

void StartObserver(Observer::Configuration* cfg);
void RecordCamera(Observer::Configuration* cfg);

using FrameType = cv::Mat;

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

    RecordCamera(&cfg);
    // StartObserver(&cfg);
}

void StartObserver(Observer::Configuration* cfg) {
    Observer::ObserverCentral<FrameType> observer(*cfg);
    observer.Start();

    std::this_thread::sleep_for(std::chrono::hours(30));
}

void RecordCamera(Observer::Configuration* cfg) {
    Observer::VideoSource<FrameType> source;
    Observer::VideoWriter<FrameType> writer;

    source.Open(cfg->camerasConfiguration[0].url);

    FrameType frame;

    source.GetNextFrame(frame);

    auto time = Observer::SpecialFunctions::GetCurrentTime();
    const std::string file = "./" + time + ".mp4";
    writer.Open(file, 14, writer.GetDefaultCodec(),
                Observer::ImageTransformation<FrameType>::GetSize(frame));

    Observer::Timer<std::chrono::seconds> timer;

    Observer::ImageDisplay<FrameType>::CreateWindow("Image");

    timer.Start();
    auto minutes = 60 * 2;
    while (timer.GetDuration() < minutes) {
        source.GetNextFrame(frame);
        writer.WriteFrame(frame);
        Observer::ImageDisplay<FrameType>::ShowImage("Image", frame);
        OBSERVER_TRACE("Left: {} seconds", minutes - timer.GetDuration());
    }

    Observer::ImageDisplay<FrameType>::DestroyWindow("Image");
    writer.Close();
    source.Close();

    OBSERVER_INFO("Recording saved at '{}'", file);
}