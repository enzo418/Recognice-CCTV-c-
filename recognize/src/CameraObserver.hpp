#pragma once

#include "OpencvVideoSource.hpp"
#include "OpencvVideoWriter.hpp"

#include "CameraConfiguration.hpp"
#include "VideoValidator.hpp"
#include "Timer.hpp"
#include "FrameProcessor.hpp"
#include "ThresholdManager.hpp"
#include "BaseObserverPattern.hpp"

// CameraEventSubscriber
#include "NotificationsController.hpp"

// FrameEventSubscriber
#include "FrameDisplay.hpp"

#include <string>
#include <opencv2/opencv.hpp>

// std::optional
#include <optional>

// std::unique_ptr
#include <memory>

namespace Observer
{
    class ThresholdEventSubscriber : public ISubscriber<CameraConfiguration*, double> {
        virtual void update(CameraConfiguration*, double) = 0;
    };


    /**
     * @brief Observes a camera and publish events
     * when movement is detected
     */
    class CameraObserver
    {
        public:
            // TODO: The configuration should be passed on Start not on create if
            // we allow to use the same camera instance to use after Stop
            CameraObserver(CameraConfiguration* configuration /**, CameraObserverBehaviour behaviour **/);

            void Start();

            void Stop();

            void SubscribeToCameraEvents(CameraEventSubscriber* subscriber);
            void SubscribeToFramesUpdate(FrameEventSubscriber* subscriber);
            void SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber);

        private:
            // camera configuration
            CameraConfiguration* cfg;

            // is the camera running
            bool running;

            // current change threshold
            double changeThreshold;

            // video source
            OpencvVideoSource source;

            // video output
            OpencvVideoWritter writer;

            std::unique_ptr<VideoValidator> validator;

            // timer to get a new frame every x ms
            Timer<std::chrono::milliseconds> timerFrames;

            FrameProcessor frameProcessor;

            ThresholdManager thresholdManager;

            Publisher<int, cv::Mat> framePublisher;

            Publisher<CameraConfiguration*, double> thresholdPublisher;

            Publisher<CameraConfiguration*, std::vector<cv::Mat>> cameraEventsPublisher;

        protected:
            void ProcessFrame(cv::Mat &frame);

            void ChangeDetected();
    };

} // namespace Observer
