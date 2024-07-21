#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

#include "observer/Blob/BlobDetector/BlobDetector.hpp"
#include "observer/Blob/Contours/ContoursDetector.hpp"
#include "observer/Blob/Contours/ContoursTypes.hpp"
#include "observer/Blob/FramesProcessor/FrameContextualizer.hpp"
#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Timer.hpp"

namespace Web::Utils {
    struct BufferData {
        std::vector<Observer::Frame> buffer;
        double fps;
    };

    struct DetectionResults {
        Observer::VideoContours contours;
        std::vector<Observer::Blob> blobs;
        std::vector<Observer::Frame> diffFrames;

        double contours_detection_time_us;
        double blob_detection_time_us;
    };

    BufferData inline ReadBufferFromCamera(const std::string& uri, int delay,
                                           int duration,
                                           Observer::Size resize) {
        std::vector<Observer::Frame> frames;

        // TODO: Get from pool
        Observer::VideoSource cap;

        cap.Open(uri);

        double fps = cap.GetFPS();

        if (!cap.isOpened()) {
            throw std::runtime_error("Could not open the camera");
        }

        // cap.set(cv::CAP_PROP_POS_MSEC, videostart * 1000);

        frames.reserve(duration * fps);

        Observer::Frame frame;

        Observer::Timer<std::chrono::seconds> bufferStartTime(false);

        OBSERVER_TRACE("Skipping start time");
        bool firstValidFrame = false;
        while (!firstValidFrame || bufferStartTime.GetDuration() < delay) {
            if (cap.GetNextFrame(frame)) {
                firstValidFrame = true;

                if (!bufferStartTime.Started()) bufferStartTime.Start();
                OBSERVER_TRACE("Skip left: {}",
                               delay - bufferStartTime.GetDuration());
            }
        }

        Observer::Timer<std::chrono::seconds> bufferTimer(true);
        OBSERVER_TRACE("Filling buffer");

        while (bufferTimer.GetDuration() < duration) {
            if (cap.GetNextFrame(frame)) {
                if (!resize.empty()) frame.Resize(resize);

                frames.push_back(frame.Clone());

                OBSERVER_TRACE("Time left: {}",
                               duration - bufferTimer.GetDuration());
            }
        }

        return BufferData {.buffer = std::move(frames), .fps = fps};
    }

    /**
     * @brief Stores the frames into a format that supports multiple images,
     * TIFF.
     *
     * @param frames frames
     * @param dstPathWithTiffExt tiff file path
     */
    void inline SaveBuffer(std::vector<Observer::Frame>& frames,
                           const std::string& dstPathWithTiffExt) {
        if (!dstPathWithTiffExt.ends_with(".tiff")) {
            throw std::logic_error("Extension must be tiff");
        }

        Observer::ImageIO::Get().SaveImages(
            dstPathWithTiffExt, frames,
            // use LZW, Lossless
            {Observer::ImageWriteFlags::USE_TIFF_COMPRESSION, 5,
             // 300 DPI
             Observer::ImageWriteFlags::USE_TIFF_RESUNIT, 300});
    }

    std::vector<Observer::Frame> inline ReadBufferFromFile(
        const std::string& tiffFilePath) {
        std::vector<Observer::Frame> frames;
        Observer::ImageIO::Get().ReadImages(tiffFilePath, frames);
        return frames;
    }

    DetectionResults inline DetectBlobs(Observer::CameraConfiguration& camera,
                                        std::vector<Observer::Frame>& frames) {
        Observer::ThresholdParams threshParams =
            camera.blobDetection.thresholdParams;

        Observer::ContoursFilter filter = camera.blobDetection.contoursFilters;

        Observer::BlobDetectorParams detectorParams =
            camera.blobDetection.blobDetectorParams;

        Observer::BlobFilters filtersBlob = camera.blobDetection.blobFilters;

        Observer::FrameContextualizer contextBuilder(threshParams);
        Observer::ContoursDetector contoursDetector(threshParams, filter);
        Observer::BlobDetector detector(detectorParams, filtersBlob,
                                        contoursDetector);

        contoursDetector.SetScale(frames[0].GetSize());

        Observer::Timer<std::chrono::microseconds> timer;
        std::vector<Observer::Blob> blobs;

        timer.Start();

        auto diffFrames = contextBuilder.GenerateDiffFrames(frames);
        auto contours = contoursDetector.FindContoursFromDiffFrames(diffFrames);

        auto took_contours = timer.GetDurationAndRestart();

        blobs = detector.FindBlobs(contours);

        auto took_detection = timer.GetDurationAndRestart();

        return {.contours = contours,
                .blobs = blobs,
                .diffFrames = diffFrames,
                .contours_detection_time_us = took_contours,
                .blob_detection_time_us = took_detection};
    }
}  // namespace Web::Utils