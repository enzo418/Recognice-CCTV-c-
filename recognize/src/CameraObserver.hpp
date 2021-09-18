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

        public:
            CameraObserver(const std::string& url, CameraConfiguration configuration /**, CameraObserverBehaviour behaviour **/);

            void Start();

            void Stop();

        protected:
            void ProcessFrame(cv::Mat &frame);
    };

} // namespace Observer
