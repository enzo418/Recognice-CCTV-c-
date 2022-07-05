#pragma once

#include "BufferedSource.hpp"
#include "Configuration/CameraConfiguration.hpp"
#include "FrameProcessor.hpp"
#include "IVideoSource.hpp"
#include "IVideoWriter.hpp"
#include "ThresholdManager.hpp"
#include "VideoBuffer.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Camera/IThresholdEventSubscriber.hpp"
#include "observer/Pattern/ObserverBasics.hpp"
#include "observer/Timer.hpp"

// CameraEventSubscriber
#include "FrameDisplay.hpp"
#include "IVideoSource.hpp"
#include "IVideoWriter.hpp"
#include "NotificationsController.hpp"
#include "observer/Functionality.hpp"

// FrameEventSubscriber
#include <chrono>
#include <iostream>
#include <string>

// std::optional
#include <optional>

// std::unique_ptr
#include <memory>
#include <thread>

namespace Observer {
    /**
     * @brief Observes a camera and publish events
     * when movement is detected
     */
    class CameraObserver : public Functionality {
       public:
        /* TODO: The configuration should be passed on Start not on create if
         * we allow to use the same camera instance to use after Stop
         **/
        explicit CameraObserver(CameraConfiguration* configuration);

        void SubscribeToFramesUpdate(IFrameSubscriber* subscriber);

        virtual void SubscribeToCameraEvents(
            ICameraEventSubscriber* subscriber) = 0;

        virtual void SubscribeToThresholdUpdate(
            IThresholdEventSubscriber* subscriber) = 0;

       protected:
        virtual void ProcessFrame(Frame& frame) = 0;

        void InternalStart() override final;

       protected:
        // camera configuration
        CameraConfiguration* cfg;

        // video source
        BufferedSource source;

        // video output
        VideoWriter writer;

        Publisher<int, Frame> framePublisher;
    };
}  // namespace Observer
