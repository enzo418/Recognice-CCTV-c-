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

void LiveTestBlobDetection(Observer::Configuration* cfg);

using FrameType = cv::Mat;

using namespace Observer;

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file, "
        "the configuration of the first camera will be used}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    int start = 0;
    int end = 0;
    std::string blobDetectionFile;

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

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    LiveTestBlobDetection(&cfg);
}

void LiveTestBlobDetection(Observer::Configuration* cfg) {
    ImageDisplay<FrameType>::CreateWindow("image");

    OBSERVER_ASSERT(!cfg->camerasConfiguration.empty(),
                    "There should be at least 1 camera.");

    auto camera = cfg->camerasConfiguration[0];

    auto videouri = camera.url;

    // Get the video
    Observer::VideoSource<FrameType> cap;

    cap.Open(videouri);

    if (!cap.isOpened()) {
        OBSERVER_CRITICAL("Couldn't connect to {0}", videouri);
        return;
    }

    auto fps = cap.GetFPS();
    OBSERVER_INFO("FPS: {0}", fps);

    ThresholdingParams param = camera.blobDetection.thresholdingParams;

    ContoursFilter filter = camera.blobDetection.contoursFilters;

    BlobDetectorParams detectorParams = camera.blobDetection.blobDetectorParams;

    BlobFilters filtersBlob = camera.blobDetection.blobFilters;

    ContoursDetector<FrameType> contoursDetector(param, filter);
    BlobDetector<FrameType> detector(detectorParams, filtersBlob,
                                     contoursDetector);

    Size resizeSize = param.Resize.size;

    int iFrame = 0;
    int max = std::numeric_limits<int>::max();
    FrameType frame;

    Timer<std::chrono::microseconds> timer;
    while (true) {
        if (cap.GetNextFrame(frame)) {
            ImageTransformation<FrameType>::Resize(frame, frame, resizeSize);

            auto contours = contoursDetector.FindContours(frame);
            double duration_contours = timer.GetDurationAndRestart();

            auto blobs = detector.FindBlobs(contours);
            double duration_blob = timer.GetDurationAndRestart();

            BlobGraphics<FrameType>::DrawBlobs(frame, blobs, 0);

            double wait =
                (1000.0 / fps) - (duration_contours + duration_blob) / 1000;
            wait = wait <= 5 ? 10 : wait;

            OBSERVER_TRACE(
                "Detection Took: {0} μs ({1} ms) | Blob detection took: {2} μs "
                "({3} ms) wait: {4}",
                duration_contours, duration_contours / 1000, duration_blob,
                duration_blob / 1000, wait);

            ImageDisplay<FrameType>::ShowImage("image", frame);

            std::this_thread::sleep_for(std::chrono::milliseconds((int)wait));

            iFrame++;
        }
    }

    ImageDisplay<FrameType>::DestroyWindow("image");
}