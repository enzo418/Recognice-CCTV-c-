#include <chrono>
#include <filesystem>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "../Observer/Implementations/opencv/BlobGraphics.hpp"
#include "../Observer/Implementations/opencv/ImageDisplay.hpp"
#include "../Observer/Implementations/opencv/ImagePersistence.hpp"
#include "../Observer/Implementations/opencv/ImageProcessing.hpp"
#include "../Observer/Implementations/opencv/ImageTransformation.hpp"
#include "../Observer/Implementations/opencv/VideoSource.hpp"
#include "../Observer/Implementations/opencv/VideoWriter.hpp"
#include "../Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../Observer/src/Domain/ObserverCentral.hpp"
#include "../Observer/src/Log/log.hpp"
#include "src/Blob/BlobDetector/BlobDetector.hpp"
#include "src/Domain/Configuration/Configuration.hpp"
#include "src/Domain/VideoSource.hpp"
#include "src/Domain/VideoWriter.hpp"
#include "src/ImageDisplay.hpp"
#include "src/ImageTransformation.hpp"
#include "src/Timer.hpp"
#include "src/Utils/SpecialFunctions.hpp"

void StartObserver(Observer::Configuration* cfg);
void RecordCamera(Observer::Configuration* cfg, int minutes);

void LiveTestBlobDetection(const std::string& file);
void TestBlobDetection(const std::string& file, int start, int end);

using FrameType = cv::Mat;

using namespace Observer;

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file}"
        "{mode              | recognizer   | Observer mode "
        "[recording|recognizer|blob|blob_live]}"
        "{recording_minutes | 2   | When mode=recording, it will record the "
        "videos "
        "for this long}"
        "{blobDetectionFile | | path to video where blob detection will be "
        "done.  }"
        "{start |   0   | blob: skips seconds from the start of the video}"
        "{end |   0   | blob: stop processing video until this seconds}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string pathConfig;
    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    int recognizerMode = RecognizerMode::RECOGNIZER;
    int recordingMinutes = 2;
    int start = 0;
    int end = 0;
    std::string blobDetectionFile;
    if (parser.has("mode")) {
        auto mode = parser.get<std::string>("mode");
        std::cout << "mode: " << mode << std::endl;

        if (mode == "blob") {
            recognizerMode = RecognizerMode::BLOB;
        } else if (mode == "blob_live") {
            recognizerMode = RecognizerMode::BLOB_LIVE;
        } else if (mode == "recording") {
            recognizerMode = RecognizerMode::RECORDING;
        }

        if (parser.has("recording_minutes")) {
            recordingMinutes = parser.get<int>("recording_minutes");
        }

        if (parser.has("start")) {
            start = parser.get<int>("start");
        }

        if (parser.has("end")) {
            end = parser.get<int>("end");
        }

        if (parser.has("blobDetectionFile")) {
            blobDetectionFile = parser.get<std::string>("blobDetectionFile");
        }
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    if (recognizerMode == RecognizerMode::RECOGNIZER) {
        StartObserver(&cfg);
    } else if (recognizerMode == RecognizerMode::RECORDING) {
        RecordCamera(&cfg, recordingMinutes);
    } else if (recognizerMode == RecognizerMode::BLOB) {
        TestBlobDetection(blobDetectionFile, start, end);
    } else if (recognizerMode == RecognizerMode::BLOB_LIVE) {
        LiveTestBlobDetection(blobDetectionFile);
    }
}

void StartObserver(Observer::Configuration* cfg) {
    Observer::ObserverCentral<FrameType> observer(*cfg);
    observer.Start();

    std::this_thread::sleep_for(std::chrono::hours(30));
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

void TestBlobDetection(const std::string& file, int start, int end) {
    cv::namedWindow("image");
    cv::startWindowThread();

    auto videouri = file;
    auto videostart = start;
    auto videoend = end;

    std::vector<cv::Mat> frames;
    // 16 FPS
    frames.reserve((videoend - videostart) * 16);

    // Get the video
    cv::VideoCapture cap(videouri);

    if (!cap.isOpened()) {
        OBSERVER_CRITICAL("Couldn't open the file {0}", file);
        return;
    }

    cap.set(cv::CAP_PROP_POS_MSEC, videostart * 1000);

    auto fps = cap.get(cv::CAP_PROP_FPS);

    cv::Size frameSize = cv::Size(cap.get(cv::CAP_PROP_FRAME_WIDTH),
                                  cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::Size resizeSize = cv::Size(640, 360);

    int ten_percent_of_screen = (resizeSize.width - resizeSize.height) * 0.08;

    ThresholdingParams dayVeryAccurate = {
        .FramesBetweenDiffFrames = 3,
        .ContextFrames = 4,
        .MedianBlurKernelSize = 3,
        .GaussianBlurKernelSize = 7,
        .DilationSize = 2,
        .BrightnessAboveThreshold = 4,
        .Resize = {.size = Size(640, 360), .resize = false}};

    auto& param = dayVeryAccurate;

    ContoursFilter filter = {
        .FilterByAverageArea = true,
        .MinimumArea = ten_percent_of_screen,
    };

    BlobDetectorParams detectorParams = {
        .distance_thresh = ten_percent_of_screen,
        .similarity_threshold = 0.5,
        .blob_max_life = static_cast<int>(3 * fps),
    };

    BlobFilters filtersBlob = {.MinimumOccurrences = (int)fps};

    ContoursDetector<cv::Mat> contoursDetector(param, filter);
    BlobDetector<cv::Mat> detector(detectorParams, filtersBlob,
                                   contoursDetector);

    int iFrame = 0;
    int max = videoend;
    cv::Mat frame;

    while (cap.get(cv::CAP_PROP_POS_MSEC) / 1000 < max) {
        if (cap.read(frame)) {
            cv::resize(frame, frame, resizeSize);

            frames.push_back(frame.clone());

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

    BlobGraphics<cv::Mat>::DrawBlobs(frames, blobs);

    int i = 0;
    while (i < frames.size()) {
        frame = frames[i];

        cv::imshow("image", frame);
        cv::waitKey(62);
        i++;
    }
    std::cout << "blobs: " << blobs.size() << std::endl;

    cv::destroyAllWindows();
}

void LiveTestBlobDetection(const std::string& file) {
    cv::namedWindow("image");
    cv::startWindowThread();

    auto videouri = file;

    // Get the video
    cv::VideoCapture cap(videouri);

    if (!cap.isOpened()) {
        OBSERVER_CRITICAL("Couldn't open the file {0}", file);
        return;
    }

    auto fps = cap.get(cv::CAP_PROP_FPS);
    OBSERVER_INFO("FPS: {0}", fps);

    cv::Size frameSize = cv::Size(cap.get(cv::CAP_PROP_FRAME_WIDTH),
                                  cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::Size resizeSize = cv::Size(640, 360);

    int ten_percent_of_screen = (resizeSize.width - resizeSize.height) * 0.08;

    ThresholdingParams dayVeryAccurate = {
        .FramesBetweenDiffFrames = 6,
        .ContextFrames = 6,
        .MedianBlurKernelSize = 3,
        .GaussianBlurKernelSize = 7,
        .DilationSize = 2,
        .BrightnessAboveThreshold = 4,
        .Resize = {.size = Size(640, 360), .resize = false}};

    auto& param = dayVeryAccurate;

    ContoursFilter filter = {
        .FilterByAverageArea = true,
        .MinimumArea = ten_percent_of_screen,
    };

    BlobDetectorParams detectorParams = {
        .distance_thresh = ten_percent_of_screen * 2,
        .similarity_threshold = 0.5,
        .blob_max_life = static_cast<int>(3 * fps),
    };

    BlobFilters filtersBlob = {.MinimumOccurrences =
                                   std::numeric_limits<int>::min()};

    ContoursDetector<cv::Mat> contoursDetector(param, filter);
    BlobDetector<cv::Mat> detector(detectorParams, filtersBlob,
                                   contoursDetector);

    int iFrame = 0;
    int max = std::numeric_limits<int>::max();
    cv::Mat frame;

    Timer<std::chrono::microseconds> timer;
    while (true) {
        if (cap.read(frame)) {
            cv::resize(frame, frame, resizeSize);

            auto contours = contoursDetector.FindContours(frame);
            double duration_contours = timer.GetDurationAndRestart();

            auto blobs = detector.FindBlobs(contours);
            double duration_blob = timer.GetDurationAndRestart();

            BlobGraphics<cv::Mat>::DrawBlobs(frame, blobs, 0);

            double wait =
                (1000.0 / fps) - (duration_contours + duration_blob) / 1000;
            wait = wait <= 5 ? 10 : wait;

            OBSERVER_TRACE(
                "Detection Took: {0} μs ({1} ms) | Blob detection took: {2} μs "
                "({3} ms) wait: {4}",
                duration_contours, duration_contours / 1000, duration_blob,
                duration_blob / 1000, wait);

            cv::imshow("image", frame);
            cv::waitKey(wait);

            iFrame++;
        }
    }
    cv::destroyAllWindows();
}