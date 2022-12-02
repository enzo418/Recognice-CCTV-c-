#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"

void TestBlobDetection(Observer::Configuration* cfg, int start, int end);

using namespace Observer;

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file, "
        "the configuration of the first camera will be used}"
        "{start |   0   | blob: skips seconds from the start of the video}"
        "{end |   0   | blob: stop processing video until this seconds}";
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

    int start = 0;
    int end = 0;
    std::string blobDetectionFile;
    if (parser.has("start")) {
        start = parser.get<int>("start");
    }

    if (parser.has("end")) {
        end = parser.get<int>("end");
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg =
        Observer::ConfigurationParser::ConfigurationFromJsonFile(pathConfig);

    TestBlobDetection(&cfg, start, end);
}

void TestBlobDetection(Observer::Configuration* cfg, int videostart,
                       int videoend) {
    ImageDisplay::Get().CreateWindow("image");

    OBSERVER_ASSERT(!cfg->cameras.empty(),
                    "There should be at least 1 camera.");

    auto camera = cfg->cameras[0];

    auto videouri = camera.url;

    // Get the video
    Observer::VideoSource cap;
    cap.Open(videouri);

    std::vector<Frame> frames;

    auto fps = cap.GetFPS();

    // 16 FPS
    frames.reserve((videoend - videostart) * fps);

    if (!cap.isOpened()) {
        OBSERVER_CRITICAL("Couldn't open the file {0}", videouri);
        return;
    }

    // cap.set(cv::CAP_PROP_POS_MSEC, videostart * 1000);

    ThresholdingParams param = camera.blobDetection.thresholdingParams;

    ContoursFilter filter = camera.blobDetection.contoursFilters;

    BlobDetectorParams detectorParams = camera.blobDetection.blobDetectorParams;

    BlobFilters filtersBlob = camera.blobDetection.blobFilters;

    ContoursDetector contoursDetector(param, filter);
    BlobDetector detector(detectorParams, filtersBlob, contoursDetector);

    Size resizeSize = param.Resize.size;

    int iFrame = 0;
    int max = std::numeric_limits<int>::max();
    Frame frame;

    Timer<std::chrono::seconds> bufferStartTime(true);

    while (bufferStartTime.GetDuration() < max) {
        if (bufferStartTime.GetDuration() >= videostart &&
            cap.GetNextFrame(frame)) {
            frame.Resize(resizeSize);

            frames.push_back(frame.Clone());

            iFrame++;
        }
    }

    // for (int i = 0; i < contours.size(); i++) {
    //     cv::drawContours(frames[i], contours[i], -1, cv::Scalar(0, 0, 255));
    // }
    Timer<std::chrono::microseconds> timer;
    std::vector<Observer::Blob> blobs;

    timer.Start();
    auto conts = contoursDetector.FindContours(frames);

    auto took_contours = timer.GetDurationAndRestart();

    OBSERVER_TRACE("Contours detection took: {0} μs ({1} ms)", took_contours,
                   took_contours / 1000);

    blobs = detector.FindBlobs(conts);

    auto took_detection = timer.GetDurationAndRestart();
    OBSERVER_TRACE("Detection Took: {0} μs ({1} ms)", took_detection,
                   took_detection / 1000);

    ImageDrawBlob::Get().DrawBlobs(frames, blobs);

    int i = 0;
    while (i < frames.size()) {
        frame = frames[i];

        ImageDisplay::Get().ShowImage("image", frame);

        double wait = (1000.0 / fps) - (took_contours + took_detection) / 1000;
        wait = wait <= 5 ? 10 : wait;

        std::this_thread::sleep_for(std::chrono::milliseconds((int)wait));

        i++;
    }
    std::cout << "blobs: " << blobs.size() << std::endl;

    ImageDisplay::Get().DestroyWindow("image");
}