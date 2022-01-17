#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "../../Observer/Implementations/opencv/Implementation.hpp"
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

    int seconds = 2;
    std::string blobDetectionFile;

    if (parser.has("seconds")) {
        seconds = parser.get<int>("seconds");
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    RecordCamera(&cfg, seconds);
}

void RecordCamera(Observer::Configuration* cfg, int pSeconds) {
    auto camcfg = &cfg->camerasConfiguration[0];
    Observer::VideoSource<FrameType> source;

    source.Open(cfg->camerasConfiguration[0].url);

    if (!source.isOpened()) {
        OBSERVER_ERROR("Could not open the camera.");
        return;
    }

    std::vector<FrameType> frames;
    frames.reserve(source.GetFPS() * pSeconds);

    Observer::Timer<std::chrono::seconds> timer;

    Observer::ImageDisplay<FrameType>::CreateWindow("Image");

    FrameType frame;
    int delay = source.GetFPS() * 27;
    while (delay--) {
        source.GetNextFrame(frame);
    }

    timer.Start();
    int i = 0;
    // timer.GetDuration() < pSeconds
    while (i < source.GetFPS() * pSeconds) {
        source.GetNextFrame(frame);

        ImageTransformation<FrameType>::Resize(frame, frame, Size(640, 360));

        frames.push_back(frame.clone());

        Observer::ImageDisplay<FrameType>::ShowImage("Image", frame);

        OBSERVER_TRACE("Left: {} seconds", pSeconds - timer.GetDuration());
        i++;
        // cv::waitKey(1000.0 / source.GetFPS());
        // cv::waitKey(1000.0 / source.GetFPS());
    }

    // Observer::ImageDisplay<FrameType>::DestroyWindow("Image");
    source.Close();

    ContoursDetector<FrameType> contoursDetector(
        camcfg->blobDetection.thresholdingParams,
        camcfg->blobDetection.contoursFilters);

    // const double factor = ((double)config->resizeNotifications.video /
    // 100.0);
    Size displaySize =
        camcfg->blobDetection.thresholdingParams.Resize.resize
            ? camcfg->blobDetection.thresholdingParams.Resize.size
            : ImageTransformation<FrameType>::GetSize(frames[0]);

    contoursDetector.SetScale(displaySize);

    VideoContours cts = contoursDetector.FindContours(frames);

    // VideoContours filtered = contoursDetector.FilterContours(cts);

    // contoursDetector.ScaleContours(cts, displaySize);

    // std::vector<Rect> realAreas;
    // auto ref = camcfg->blobDetection.contoursFilters.ignoredAreas.reference;
    // double scaleX = (double)displaySize.width / ref.width;
    // double scaleY = (double)displaySize.height / ref.height;

    // for (auto& rect :
    //      camcfg->blobDetection.contoursFilters.ignoredAreas.areas) {
    //     realAreas.push_back(Rect(rect.x * scaleX, rect.y * scaleY,
    //                              rect.width * scaleX, rect.height * scaleY));
    // }

    for (int j = 0; j < frames.size(); j++) {
        auto frame = frames[j].clone();

        ImageTransformation<FrameType>::Resize(frame, frame, displaySize);

        // for (auto& rect :
        //      camcfg->blobDetection.contoursFilters.ignoredAreas.areas) {
        //     cv::rectangle(frame, rect, cv::Scalar(255, 0, 255));
        // }

        for (auto& rect : contoursDetector.filters.ignoredAreas.areas) {
            cv::rectangle(frame, rect, cv::Scalar(20, 255, 255));
        }

        /*for (auto& set : contoursDetector.filters.ignoredSets.sets) {
            Point first = set[0];
            Point last = set[0];

            for (auto& point : set) {
                cv::circle(frame, point, 2, cv::Scalar(20, 255, 255));
                cv::line(frame, last, point, cv::Scalar(20, 255, 255));
                last = point;
            }

            cv::line(frame, last, first, cv::Scalar(20, 255, 255));
        }*/

        for (int i = 0; i < cts[j].size(); i++) {
            auto rect = BoundingRect(cts[j][i]);
            cv::rectangle(frame, rect, cv::Scalar(0, 150, 255));
        }

        // for (int i = 0; i < filtered[j].size(); i++) {
        //     auto rect = BoundingRect(filtered[j][i]);
        //     cv::rectangle(frame, rect, cv::Scalar(0, 0, 255));
        // }

        Observer::ImageDisplay<FrameType>::ShowImage("Image", frame);
        cv::waitKey(1000.0 / source.GetFPS());
    }
}
