#include <chrono>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <optional>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>

#include "observer/AsyncInference/DetectorClient.hpp"
#include "observer/Blob/Contours/ContoursTypes.hpp"
#include "observer/Domain/Classification/BlobClassification.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"
#include "observer/IFrame.hpp"
#include "observer/IImageIO.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
#include "observer/Log/log.hpp"
#include "observer/ScalarVector.hpp"

using namespace Observer;

/* ------------ BUFFER METADATA INPUT/OUTPUT ------------ */
struct BufferMetaData {
    double fps;
    Size storedSize {640, 360};
} fileBufferData;

void inline readBufferMetadata(const std::string& metadataPath,
                               BufferMetaData& out) {
    std::ifstream input(metadataPath, std::ios::binary);
    input.read((char*)&out, sizeof(BufferMetaData));
}

void inline writeBufferMetadata(const std::string& metadataPath,
                                const BufferMetaData& metadata) {
    std::ofstream out(metadataPath, std::ios::binary);

    out.write((char*)&metadata, sizeof(BufferMetaData));
}

/* ------------- READ BUFFER FROM LIVE/FILE ------------- */
std::vector<Frame> ReadBufferFromCamera(
    Observer::CameraConfiguration& cameraCfg, int start, int end,
    bool saveIntoFile);

std::vector<Frame> ReadBufferFromFile(const std::string& tiffFilePath);

void inline SaveBuffer(std::vector<Frame>& frames,
                       std::optional<std::string> pBasePath = std::nullopt) {
    const std::string basePath =
        pBasePath.value_or(std::string("./images_CMP5_DPI300_") +
                           std::to_string(std::hash<std::thread::id> {}(
                               std::this_thread::get_id())));

    const std::string imagePath = basePath + ".tiff";
    const std::string dataFilePath = basePath + ".bin";

    OBSERVER_TRACE("Saving buffer as {} ...", imagePath);

    ImageIO::Get().SaveImages(imagePath, frames,
                              // use LZW, Lossless
                              {ImageWriteFlags::USE_TIFF_COMPRESSION, 5,
                               // 300 DPI
                               ImageWriteFlags::USE_TIFF_RESUNIT, 300});

    writeBufferMetadata(dataFilePath, fileBufferData);

    OBSERVER_TRACE("Done!");
}

/* ------------------- BLOB DETECTION ------------------- */
struct DetectionResults {
    VideoContours contours;
    std::vector<Blob> blobs;
    std::vector<Frame> diffFrames;
    BlobClassifications classifications;

    double contours_detection_time_us;
    double blob_detection_time_us;
};

DetectionResults DetectBlobs(Observer::CameraConfiguration& cameraCfg,
                             std::vector<Frame>& buffer,
                             AsyncInference::DetectorClient* detectionClient);

void DisplayResults(DetectionResults& results, std::vector<Frame>& buffer);

void TestThatBlobDetectionIsDeterministic(
    Observer::CameraConfiguration& cameraCfg, std::vector<Frame>& buffer);

enum RecognizerMode { RECOGNIZER, RECORDING, BLOB, BLOB_LIVE };

int main(int argc, char** argv) {
    const std::string keys =
        "{help h            |              | show help message}"
        "{config_path       | ./config.yml | path of the configuration file, "
        "the configuration of the first camera will be used}"
        "{start |   0   | blob: skips seconds from the start of the video}"
        "{end |   0   | blob: stop processing video until this seconds}"
        "{save-frames | true | save the captured frames into a tiff file}"
        "{use-file-as-buffer | | use tiff image as the buffer (.tiff path) }"
        "{classify | false | classify the blobs using the inference server}";

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
    bool saveIntoFile = false;
    std::string useImagesFileAsBuffer;

    if (parser.has("start")) {
        start = parser.get<int>("start");
    }

    if (parser.has("end")) {
        end = parser.get<int>("end");
    }

    if (parser.has("save-frames")) {
        saveIntoFile = parser.get<bool>("save-frames");
    }

    if (parser.has("use-file-as-buffer")) {
        useImagesFileAsBuffer = parser.get<std::string>("use-file-as-buffer");
    }

    OBSERVER_INIT_INSTRUMENTATION();
    OBSERVER_DECLARE_THREAD("Main");

    // initialize logger
    Observer::LogManager::Initialize();
    OBSERVER_INFO("Hi");

    std::cout << "file: " << pathConfig
              << " curr dir: " << std::filesystem::current_path().string()
              << std::endl;

    auto cfg =
        Observer::ConfigurationParser::ConfigurationFromJsonFile(pathConfig);

    OBSERVER_ASSERT(!cfg.cameras.empty(), "There should be at least 1 camera.");

    auto& camera = cfg.cameras[0];

    AsyncInference::DetectorClient* detectionClient = nullptr;

    if (parser.has("classify") && parser.get<bool>("classify")) {
        detectionClient = new AsyncInference::DetectorClient(
            camera.objectDetectionValidatorConfig.serverAddress);
    }

    std::vector<Frame> buffer;
    if (useImagesFileAsBuffer.empty()) {
        buffer = ReadBufferFromCamera(camera, start, end, saveIntoFile);

        TestThatBlobDetectionIsDeterministic(camera, buffer);
    } else {
        buffer = ReadBufferFromFile(useImagesFileAsBuffer);
    }

    auto result = DetectBlobs(camera, buffer, detectionClient);

    OBSERVER_TRACE(
        "Contours detection took: {0} μs ({1} ms) - Total contours {2}",
        result.contours_detection_time_us,
        result.contours_detection_time_us / 1000, result.contours.size());

    OBSERVER_TRACE("Detection Took: {0} μs ({1} ms) - Total blobs: {2}",
                   result.blob_detection_time_us,
                   result.blob_detection_time_us / 1000, result.blobs.size());

    DisplayResults(result, buffer);

    OBSERVER_STOP_INSTRUMENTATION();
}

std::vector<Frame> ReadBufferFromCamera(
    Observer::CameraConfiguration& cameraCfg, int videoStart, int videoEnd,
    bool saveIntoFile) {
    std::vector<Frame> frames;
    auto videoUri = cameraCfg.url;

    // Get the video
    Observer::VideoSource cap;
    cap.Open(videoUri);

    fileBufferData.fps = cap.GetFPS();

    if (!cap.isOpened()) {
        OBSERVER_CRITICAL("Couldn't open the file {0}", videoUri);
        throw std::runtime_error("Could not open the camera");
    }

    // cap.set(cv::CAP_PROP_POS_MSEC, videostart * 1000);

    frames.reserve((videoEnd - videoStart) * fileBufferData.fps);

    Frame frame;

    Timer<std::chrono::seconds> bufferStartTime(false);

    OBSERVER_TRACE("Skipping start time");
    bool firstValidFrame = false;
    while (!firstValidFrame || bufferStartTime.GetDuration() < videoStart) {
        if (cap.GetNextFrame(frame)) {
            firstValidFrame = true;

            if (!bufferStartTime.Started()) bufferStartTime.Start();
            OBSERVER_TRACE("Skip left: {}",
                           videoStart - bufferStartTime.GetDuration());
        }
    }

    Timer<std::chrono::seconds> bufferTimer(true);
    OBSERVER_TRACE("Filling buffer");

    while (bufferTimer.GetDuration() < videoEnd) {
        if (bufferTimer.GetDuration() >= videoStart &&
            cap.GetNextFrame(frame)) {
            frame.Resize(fileBufferData.storedSize);

            frames.push_back(frame.Clone());

            OBSERVER_TRACE("Time left: {}",
                           videoEnd - bufferTimer.GetDuration());
        }
    }

    if (saveIntoFile) {
        const std::string basePath =
            std::string("./images_CMP5_DPI300_") +
            std::to_string(
                std::hash<std::thread::id> {}(std::this_thread::get_id()));
        const std::string imagePath = basePath + ".tiff";
        const std::string dataFilePath = basePath + ".bin";

        OBSERVER_TRACE("Saving buffer as {} ...", imagePath);

        ImageIO::Get().SaveImages(imagePath, frames,
                                  // use LZW, Lossless
                                  {ImageWriteFlags::USE_TIFF_COMPRESSION, 5,
                                   // 300 DPI
                                   ImageWriteFlags::USE_TIFF_RESUNIT, 300});

        writeBufferMetadata(dataFilePath, fileBufferData);

        OBSERVER_TRACE("Done!");
    }

    return frames;
}

std::vector<Frame> ReadBufferFromFile(const std::string& tiffFilePath) {
    std::vector<Frame> frames;
    ImageIO::Get().ReadImages(tiffFilePath, frames);

    std::string fileBufferDataPath(tiffFilePath.begin(),
                                   tiffFilePath.end() - 5);

    readBufferMetadata(fileBufferDataPath + ".bin", fileBufferData);

    return frames;
}

DetectionResults DetectBlobs(Observer::CameraConfiguration& camera,
                             std::vector<Frame>& frames,
                             AsyncInference::DetectorClient* detectionClient) {
    ThresholdParams threshParams = camera.blobDetection.thresholdParams;

    ContoursFilter filter = camera.blobDetection.contoursFilters;

    BlobDetectorParams detectorParams = camera.blobDetection.blobDetectorParams;

    BlobFilters filtersBlob = camera.blobDetection.blobFilters;

    FrameContextualizer contextBuilder(threshParams);
    ContoursDetector contoursDetector(threshParams, filter);
    BlobDetector detector(detectorParams, filtersBlob, contoursDetector);

    contoursDetector.SetScale(fileBufferData.storedSize);

    Timer<std::chrono::microseconds> timer;
    std::vector<Observer::Blob> blobs;

    timer.Start();

    auto diffFrames = contextBuilder.GenerateDiffFrames(frames);
    auto contours = contoursDetector.FindContoursFromDiffFrames(diffFrames);

    auto took_contours = timer.GetDurationAndRestart();

    {
        OBSERVER_SCOPE("Find blobs");
        blobs = detector.FindBlobs(contours);
    }

    BlobClassifications classifications;
    if (detectionClient) {
        std::cout << "Detecting...\n";
        // send 1 frame per second (will skip > 15 frames)
        AsyncInference::SendEveryNthFrame sendStrategy(fileBufferData.fps);

        std::vector<AsyncInference::ImageDetections> detections;

        {
            OBSERVER_SCOPE("Detect objects");

            detections = detectionClient->Detect(
                frames,
                [](const auto& detection) {
                    std::cout << "\t Label: " << detection.label << std::endl;
                },
                &sendStrategy);
        }

#if 0  // show detections
        for (auto& detection : detections) {
            cv::Mat frame = frames[detection.image_index].GetInternalFrame();

            for (const auto& detection : detection.detections) {
                std::cout << "\t[" << detection.label
                          << "] Rect: " << detection.x << ", " << detection.y
                          << ", " << detection.width << ", " << detection.height
                          << std::endl;

                cv::rectangle(frame,
                              cv::Rect(detection.x, detection.y,
                                       detection.width, detection.height),
                              cv::Scalar(0, 255, 0), 2);
                cv::putText(
                    frame, detection.label, cv::Point(detection.x, detection.y),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
            }

            cv::imshow("detections", frame);
            cv::waitKey(0);
        }
#endif

        classifications = AssignObjectToBlob(blobs, detections);
    }

    auto took_detection = timer.GetDurationAndRestart();

    return {
        .contours = std::move(contours),
        .blobs = std::move(blobs),
        .diffFrames = std::move(diffFrames),
        .classifications = std::move(classifications),
        .contours_detection_time_us = took_contours,
        .blob_detection_time_us = took_detection,
    };
}

void DisplayResults(DetectionResults& results, std::vector<Frame>& frames) {
    auto& contours = results.contours;
    auto& blobs = results.blobs;
    auto& diffFrames = results.diffFrames;

    // copy the frames so we have a contours and blobs view
    std::vector<Frame> contoursFrames(frames.size());

    std::vector<Frame> blobsFrames(frames.size());

    for (size_t i = 0; i < frames.size(); i++) {
        frames[i].CopyTo(contoursFrames[i]);
        frames[i].CopyTo(blobsFrames[i]);
    }

    ImageDraw::Get().DrawContours(contoursFrames, contours,
                                  ScalarVector(80, 80, 255));

    ImageDrawBlob::Get().DrawBlobs(blobsFrames, blobs, results.classifications);

    std::array<Frame, 4> framesToStack;

    // convert diffFrame to the same color space as frame so we can stack
    // them
    for (auto& diffFrame : diffFrames) {
        diffFrame.ToColorSpace(ColorSpaceConversion::COLOR_GRAY2RGB);
    }

    const double wait = 1000.0 / fileBufferData.fps;

    ImageDisplay::Get().CreateWindow("image");

    Frame frame;
    size_t i = 0;
    while (true) {
        framesToStack = {frames[i], diffFrames[i], contoursFrames[i],
                         blobsFrames[i]};

        frame = ImageDisplay::Get().StackImages(&framesToStack[0], 4, 2);

        ImageDisplay::Get().ShowImage("image", frame);

        std::this_thread::sleep_for(std::chrono::milliseconds((int)wait));

        i++;

        if (i > frames.size() - 1) {
            // start again
            std::this_thread::sleep_for(std::chrono::seconds(2));
            i = 0;
        }
        break;
    }

    ImageDisplay::Get().DestroyWindow("image");
}

void TestThatBlobDetectionIsDeterministic(
    Observer::CameraConfiguration& camera,
    std::vector<Frame>& bufferFromCamera) {
    auto result1 = DetectBlobs(camera, bufferFromCamera, nullptr);
    SaveBuffer(bufferFromCamera, "deterministic_test");

    auto bufferFromFile = ReadBufferFromFile("deterministic_test.tiff");

    auto result2 = DetectBlobs(camera, bufferFromFile, nullptr);

    bool deterministic = true;

    if (result1.contours.size() != result2.contours.size()) {
        OBSERVER_WARN(
            "contours from camera and file are not the same, camera has {} and "
            "file has {}",
            result1.contours.size(), result2.contours.size());
        deterministic = false;
    }

    if (result1.blobs.size() != result2.blobs.size()) {
        OBSERVER_WARN(
            "blobs from camera and file are not the same, camera has {} and "
            "file has {}",
            result1.blobs.size(), result2.blobs.size());
        deterministic = false;
    }

    if (deterministic) {
        OBSERVER_INFO(
            "Blob detection produced the same results in both, camera and file "
            "buffer.");
    }

    double delta_blob =
        abs(result1.blob_detection_time_us - result2.blob_detection_time_us) /
        1000;

    double delta_contours = abs(result1.contours_detection_time_us -
                                result2.contours_detection_time_us) /
                            1000;

    OBSERVER_INFO(
        "Delta time between the buffers:\tcontours = {}ms\tblobs = {}ms",
        delta_contours, delta_blob);
}