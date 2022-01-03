#include <chrono>
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>
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

using FrameType = cv::Mat;

using namespace Observer;

int main(int argc, char** argv) {
    std::string pathConfig;
    std::string outputConfig = "./config_ouput.yml";

    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file}"
        "{mode              | recognizer   | Observer mode "
        "[recording|recognizer]}"
        "{recording_minutes | 2   | When mode=recording, it will record the "
        "videos "
        "for this long}";

    cv::CommandLineParser parser(argc, argv, keys);

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    if (parser.has("config_path")) {
        pathConfig = parser.get<std::string>("config_path");
    }

    bool recognizerMode = true;
    int recordingMinutes = 2;
    if (parser.has("mode")) {
        recognizerMode = parser.get<std::string>("mode") == "recognizer";
        if (parser.has("recording_minutes")) {
            recordingMinutes = parser.get<int>("recording_minutes");
        }
    }

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg = Observer::ConfigurationParser::ParseYAML(pathConfig);

    cv::namedWindow("image");
    cv::startWindowThread();

    auto videouri = "./person_2.mp4";
    auto videostart = 0;
    auto videoend = 10;

    std::vector<cv::Mat> frames;
    // 16 FPS
    frames.reserve((videoend - videostart) * 16);

    // Get the video
    cv::VideoCapture cap(videouri);

    if (!cap.isOpened()) {
        std::cout << " no pudo abrir " << std::endl;
        return 0;
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

    std::vector<Observer::Blob> blobs;

    auto conts = contoursDetector.FindContours(frames);

    blobs = detector.FindBlobs(conts);

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

    // if (recognizerMode) {
    //     StartObserver(&cfg);
    // } else {
    //     RecordCamera(&cfg, recordingMinutes);
    // }
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
