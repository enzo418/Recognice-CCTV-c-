#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"

void RecordCamera(Observer::Configuration* cfg, int minutes);

using FrameType = cv::Mat;

using namespace Observer;

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file}"
        "{recording_minutes | 2   | When mode=recording, it will record the "
        "videos for this long}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    int recordingMinutes = 2;
    std::string blobDetectionFile;

    if (parser.has("recording_minutes")) {
        recordingMinutes = parser.get<int>("recording_minutes");
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg =
        Observer::ConfigurationParser::ConfigurationFromJsonFile(pathConfig);

    RecordCamera(&cfg, recordingMinutes);
}

void RecordCamera(Observer::Configuration* cfg, int pMinutesRecording) {
    Observer::VideoSource source;
    Observer::VideoWriter writer;

    source.Open(cfg->cameras[0].url);

    if (!source.isOpened()) {
        OBSERVER_ERROR("Could not open the camera.");
        return;
    }

    Frame frame;

    source.GetNextFrame(frame);

    auto time = Observer::SpecialFunctions::GetCurrentTime();
    const std::string file = "./" + time + ".mp4";
    writer.Open(file, source.GetFPS(), writer.GetDefaultCodec(),
                frame.GetSize());

    Observer::Timer<std::chrono::seconds> timer;

    Observer::ImageDisplay::Get().CreateWindow("Image");

    timer.Start();
    auto minutes = 60 * pMinutesRecording;
    while (timer.GetDuration() < minutes) {
        source.GetNextFrame(frame);
        writer.WriteFrame(frame);
        Observer::ImageDisplay::Get().ShowImage("Image", frame);
        OBSERVER_TRACE("Left: {} seconds", minutes - timer.GetDuration());
    }

    Observer::ImageDisplay::Get().DestroyWindow("Image");
    writer.Close();
    source.Close();

    OBSERVER_INFO("Recording saved at '{}'", file);
}
