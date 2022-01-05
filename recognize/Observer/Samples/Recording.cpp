#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "../../Observer/Implementations/opencv/BlobGraphics.hpp"
#include "../../Observer/Implementations/opencv/ImageDisplay.hpp"
#include "../../Observer/Implementations/opencv/ImagePersistence.hpp"
#include "../../Observer/Implementations/opencv/ImageProcessing.hpp"
#include "../../Observer/Implementations/opencv/ImageTransformation.hpp"
#include "../../Observer/Implementations/opencv/VideoSource.hpp"
#include "../../Observer/Implementations/opencv/VideoWriter.hpp"
#include "../../Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../../Observer/src/Domain/ObserverCentral.hpp"
#include "../../Observer/src/Log/log.hpp"
#include "../src/Blob/BlobDetector/BlobDetector.hpp"
#include "../src/Domain/Configuration/Configuration.hpp"
#include "../src/Domain/VideoSource.hpp"
#include "../src/Domain/VideoWriter.hpp"
#include "../src/ImageDisplay.hpp"
#include "../src/ImageTransformation.hpp"
#include "../src/Timer.hpp"
#include "../src/Utils/SpecialFunctions.hpp"

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

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    RecordCamera(&cfg, recordingMinutes);
}

void RecordCamera(Observer::Configuration* cfg, int pMinutesRecording) {
    Observer::VideoSource<FrameType> source;
    Observer::VideoWriter<FrameType> writer;

    source.Open(cfg->camerasConfiguration[0].url);

    if (!source.isOpened()) {
        OBSERVER_ERROR("Could not open the camera.");
        return;
    }

    FrameType frame;

    source.GetNextFrame(frame);

    auto time = Observer::SpecialFunctions::GetCurrentTime();
    const std::string file = "./" + time + ".mp4";
    writer.Open(file, source.GetFPS(), writer.GetDefaultCodec(),
                Observer::ImageTransformation<FrameType>::GetSize(frame));

    Observer::Timer<std::chrono::seconds> timer;

    Observer::ImageDisplay<FrameType>::CreateWindow("Image");

    timer.Start();
    auto minutes = 60 * pMinutesRecording;
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
