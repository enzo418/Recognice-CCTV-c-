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
     * @brief Consumes frames from a video source and notifies subscribers.
     */
    class CameraObserver : public Functionality {
       public:
        /* TODO: The configuration should be passed on Start not on create if
         * we allow to use the same camera instance to use after Stop
         **/
        explicit CameraObserver(CameraConfiguration* configuration);

        /* ----------------------- Events ----------------------- */
        void SubscribeToFramesUpdate(IFrameSubscriber* subscriber);

        void SubscribeToCameraEvents(ICameraEventSubscriber* subscriber);

        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

        /* ----------------------- Setters ---------------------- */

        /**
         * @brief Change the camera type .
         *
         * @param type the new camera type, note that it DOES NOT stop or start
         * the camera. Do so by calling Start/Stop.
         */
        void SetType(ECameraType type);

        /* ----------------------- Getters ---------------------- */
        /**
         * @brief Get the calculated FPS. This values is the minimum between the
         * the configurations fps and the source (camera) fps.
         *
         * Note: This value is 0 until you Start this functionality.
         *
         * @return int
         */
        int GetFPS();

        std::string GetName();

        ECameraType GetType();

       protected:
        void ProcessFrame(Frame& frame);

        void SetupDependencies();

        void ChangeDetected();

        void NewVideoBuffer();

       protected:
        void InternalStart() override final;

       protected:
        // camera type - can change during runtime.
        std::atomic<ECameraType> type;

        // camera configuration
        CameraConfiguration* cfg;

        // video source
        BufferedSource source;

        // video output
        VideoWriter writer;

        Publisher<int, Frame> framePublisher;

        // final calculated fps
        int fps {0};

       private:  // EVENTS
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

        // current change threshold
        double changeThreshold;

        FrameProcessor frameProcessor;

        std::unique_ptr<VideoBuffer> videoBufferForValidation;

        Publisher<CameraConfiguration*, std::shared_ptr<CameraEvent>>
            cameraEventsPublisher;

        ThresholdManager thresholdManager;

        // threshold publisher
        ThresholdEventPublisher thresholdPublisher;
    };
}  // namespace Observer
