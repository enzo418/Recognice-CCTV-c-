#pragma once

#include <string>
#include <opencv2/opencv.hpp>

// std::optional
#include <optional>

// std::unique_ptr
#include <memory>

#include "OpencvVideoSource.hpp"
#include "OpencvVideoWriter.hpp"

#include "CameraConfiguration.hpp"

#include "VideoValidator.hpp"

#include "Timer.hpp"

#include "FrameProcessor.hpp"

#include "ThresholdManager.hpp"

namespace Observer
{
    class CameraObserver
    {
        private:
            // is the camera running
            bool running;

            // current change threshold
            double changeThreshold;

            // video source
            OpencvVideoSource source;

            // video output
            OpencvVideoWritter writer;

            // camera configuration
            std::optional<CameraConfiguration> cfg;

            std::unique_ptr<VideoValidator> validator;

            // timer to get a new frame every x ms
            Timer<std::chrono::milliseconds> timerFrames;

            FrameProcessor frameProcessor;

            ThresholdManager thresholdManager;

        public:
            CameraObserver(CameraConfiguration configuration /**, CameraObserverBehaviour behaviour **/);

            void Start();

            void Stop();

        protected:
            void ProcessFrame(cv::Mat &frame);

            void ChangeDetected();
    };

} // namespace Observer
