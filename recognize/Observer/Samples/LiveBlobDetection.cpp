#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "observer/Domain/Classification/BlobClassification.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"

void LiveTestBlobDetection(Observer::Configuration* cfg, int cameraNumber);

using namespace Observer;

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file}"
        "{camera_number       | 0 | camera configuration to use}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    int camera_number = 0;
    int start = 0;
    int end = 0;
    std::string blobDetectionFile;

    if (parser.has("camera_number")) {
        camera_number = parser.get<int>("camera_number");
    }

    // if (parser.has("start")) {
    //     start = parser.get<int>("start");
    // }

    // if (parser.has("end")) {
    //     end = parser.get<int>("end");
    // }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg =
        Observer::ConfigurationParser::ConfigurationFromJsonFile(pathConfig);

    LiveTestBlobDetection(&cfg, camera_number);
}

void LiveTestBlobDetection(Observer::Configuration* cfg, int camera_number) {
    ImageDisplay::Get().CreateWindow("image");

    OBSERVER_ASSERT(!cfg->cameras.empty(),
                    "There should be at least 1 camera.");

    OBSERVER_ASSERT(camera_number < cfg->cameras.size(),
                    "No camera at given position. The position starts from 0.");

    auto camera = cfg->cameras[camera_number];

    auto videouri = camera.url;

    // Get the video
    Observer::VideoSource cap;

    cap.Open(videouri);

    if (!cap.isOpened()) {
        OBSERVER_CRITICAL("Couldn't connect to {0}", videouri);
        return;
    }

    auto fps = cap.GetFPS();
    OBSERVER_INFO("FPS: {0}", fps);

    ThresholdParams param = camera.blobDetection.thresholdParams;

    ContoursFilter filter = camera.blobDetection.contoursFilters;

    BlobDetectorParams detectorParams = camera.blobDetection.blobDetectorParams;

    BlobFilters filtersBlob = camera.blobDetection.blobFilters;

    ContoursDetector contoursDetector(param, filter);
    BlobDetector detector(detectorParams, filtersBlob, contoursDetector);

    Size resizeSize = param.Resize.size;

    int iFrame = 0;
    int max = std::numeric_limits<int>::max();
    Frame frame;

    Timer<std::chrono::microseconds> timer;
    while (true) {
        if (cap.GetNextFrame(frame)) {
            frame.Resize(resizeSize);

            auto contours = contoursDetector.FindContours(frame);
            double duration_contours = timer.GetDurationAndRestart();

            auto blobs = detector.FindBlobs(contours);
            double duration_blob = timer.GetDurationAndRestart();

            BlobClassifications classifications;
            ImageDrawBlob::Get().DrawBlobs(frame, blobs, classifications, 0);

            double wait =
                (1000.0 / fps) - (duration_contours + duration_blob) / 1000;
            wait = wait <= 5 ? 10 : wait;

            OBSERVER_TRACE(
                "Detection Took: {0} μs ({1} ms) | Blob detection took: {2} μs "
                "({3} ms) wait: {4}",
                duration_contours, duration_contours / 1000, duration_blob,
                duration_blob / 1000, wait);

            ImageDisplay::Get().ShowImage("image", frame);

            std::this_thread::sleep_for(std::chrono::milliseconds((int)wait));

            iFrame++;
        }
    }

    ImageDisplay::Get().DestroyWindow("image");
}