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

void RecordCamera(Observer::Configuration* cfg, int minutes);

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

    auto cfg =
        Observer::ConfigurationParser::ConfigurationFromJsonFile(pathConfig);

    RecordCamera(&cfg, seconds);
}

void RecordCamera(Observer::Configuration* cfg, int pSeconds) {
    auto camcfg = &cfg->cameras[0];
    Observer::VideoSource source;

    source.Open(cfg->cameras[0].url);

    if (!source.isOpened()) {
        OBSERVER_ERROR("Could not open the camera.");
        return;
    }

    std::vector<Frame> frames;
    frames.reserve(source.GetFPS() * pSeconds);

    Observer::Timer<std::chrono::seconds> timer;

    Observer::ImageDisplay::Get().CreateWindow("Image");

    Frame frame;
    int delay = source.GetFPS() * 27;
    while (delay--) {
        source.GetNextFrame(frame);
    }

    timer.Start();
    int i = 0;
    // timer.GetDuration() < pSeconds
    while (i < source.GetFPS() * pSeconds) {
        source.GetNextFrame(frame);

        frame.Resize(Size(640, 360));

        frames.push_back(frame.Clone());

        Observer::ImageDisplay::Get().ShowImage("Image", frame);

        OBSERVER_TRACE("Left: {} seconds", pSeconds - timer.GetDuration());
        i++;
        // cv::waitKey(1000.0 / source.GetFPS());
        // cv::waitKey(1000.0 / source.GetFPS());
    }

    // Observer::ImageDisplay<FrameType>::DestroyWindow("Image");
    source.Close();

    ContoursDetector contoursDetector(camcfg->blobDetection.thresholdingParams,
                                      camcfg->blobDetection.contoursFilters);

    // const double factor = ((double)config->resizeNotifications.video /
    // 100.0);
    Size displaySize =
        camcfg->blobDetection.thresholdingParams.Resize.resize
            ? camcfg->blobDetection.thresholdingParams.Resize.size
            : frames[0].GetSize();

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
        auto frame = frames[j].Clone();

        frame.Resize(displaySize);

        // for (auto& rect :
        //      camcfg->blobDetection.contoursFilters.ignoredAreas.areas) {
        //     cv::rectangle(frame, rect, cv::Scalar(255, 0, 255));
        // }

        for (auto& rect : contoursDetector.filters.ignoredAreas.areas) {
            cv::rectangle(frame.GetInternalFrame(), rect,
                          cv::Scalar(20, 255, 255));
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
            cv::rectangle(frame.GetInternalFrame(), rect,
                          cv::Scalar(0, 150, 255));
        }

        // for (int i = 0; i < filtered[j].size(); i++) {
        //     auto rect = BoundingRect(filtered[j][i]);
        //     cv::rectangle(frame, rect, cv::Scalar(0, 0, 255));
        // }

        Observer::ImageDisplay::Get().ShowImage("Image", frame);
        cv::waitKey(1000.0 / source.GetFPS());
    }
}
