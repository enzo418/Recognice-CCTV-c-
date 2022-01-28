#pragma once

#include "../Log/log.hpp"
#include "../Pattern/Camera/IThresholdEventSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Timer.hpp"
#include "BufferedSource.hpp"
#include "Configuration/CameraConfiguration.hpp"
#include "FrameProcessor.hpp"
#include "IVideoSource.hpp"
#include "IVideoWriter.hpp"
#include "ThresholdManager.hpp"
#include "VideoBuffer.hpp"

// CameraEventSubscriber
#include "../Functionality.hpp"
#include "FrameDisplay.hpp"
#include "IVideoSource.hpp"
#include "IVideoWriter.hpp"
#include "NotificationsController.hpp"

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

        void SubscribeToCameraEvents(ICameraEventSubscriber* subscriber);
        void SubscribeToFramesUpdate(IFrameSubscriber* subscriber);
        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

       protected:
        void ProcessFrame(Frame& frame);

        void ChangeDetected();

        void NewVideoBuffer();

        void InternalStart() override final;

       private:
        // camera configuration
        CameraConfiguration* cfg;

        // current change threshold
        double changeThreshold;

        // video source
        BufferedSource source;

        // video output
        VideoWriter writer;

        std::unique_ptr<VideoBuffer> videoBufferForValidation;

        FrameProcessor frameProcessor;

        ThresholdManager thresholdManager;

        Publisher<int, Frame> framePublisher;

        Publisher<CameraConfiguration*, CameraEvent> cameraEventsPublisher;

        /* Proxy allows us to give our subscribers not only the threshold,
         * but also the camera that updated the threshold.
         */
        class ThresholdEventPublisher : public ISubscriber<double> {
           public:
            explicit ThresholdEventPublisher(CameraConfiguration* pCfg)
                : cfg(pCfg) {}

            void subscribe(IThresholdEventSubscriber* subscriber) {
                this->thresholdPublisher.subscribe(subscriber);
            }

            void update(double thresh) override {
                this->thresholdPublisher.notifySubscribers(this->cfg, thresh);
            }

           private:
            CameraConfiguration* cfg;
            Publisher<CameraConfiguration*, double> thresholdPublisher;
        };

        // threshold publisher
        ThresholdEventPublisher thresholdPublisher;
    };
}  // namespace Observer
