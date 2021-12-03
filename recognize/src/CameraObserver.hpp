#pragma once

#include "BaseObserverPattern.hpp"
#include "CameraConfiguration.hpp"
#include "FrameProcessor.hpp"
#include "ThresholdManager.hpp"
#include "Timer.hpp"
#include "VideoBuffer.hpp"
#include "VideoSource.hpp"
#include "VideoWriter.hpp"

// CameraEventSubscriber
#include "IFunctionality.hpp"
#include "NotificationsController.hpp"

// FrameEventSubscriber
#include <opencv2/opencv.hpp>
#include <string>

#include "FrameDisplay.hpp"
#include "VideoSource.hpp"
#include "VideoWriter.hpp"

// std::optional
#include <optional>

// std::unique_ptr
#include <memory>

namespace Observer {
    class ThresholdEventSubscriber
        : public ISubscriber<CameraConfiguration*, double> {
        void update(CameraConfiguration*, double) override = 0;
    };

    /**
     * @brief Observes a camera and publish events
     * when movement is detected
     */
    template <typename TFrame>
    class CameraObserver : public IFunctionality {
       public:
        /* TODO: The configuration should be passed on Start not on create if
         * we allow to use the same camera instance to use after Stop
         **/
        explicit CameraObserver(CameraConfiguration* configuration);

        void Start() override;

        void Stop() override;

        void SubscribeToCameraEvents(CameraEventSubscriber<TFrame>* subscriber);
        void SubscribeToFramesUpdate(FrameEventSubscriber<TFrame>* subscriber);
        void SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber);

       private:
        // camera configuration
        CameraConfiguration* cfg;

        // is the camera running
        bool running;

        // current change threshold
        double changeThreshold;

        // video source
        VideoSource<TFrame> source;

        // video output
        VideoWriter<TFrame> writer;

        std::unique_ptr<VideoBuffer<TFrame>> validator;

        // timer to get a new frame every x ms
        Timer<std::chrono::milliseconds> timerFrames;

        FrameProcessor<TFrame> frameProcessor;

        ThresholdManager thresholdManager;

        Publisher<int, TFrame> framePublisher;

        Publisher<CameraConfiguration*, RawCameraEvent<TFrame>>
            cameraEventsPublisher;

        // this flag is used to know when
        // we are waiting to fill the
        // "after" buffer on validator.
        bool waitingBufferFill;

        /* Proxy allows us to give our subscribers not only the threshold,
         * but also the camera that updated the threshold.
         */
        class ProxyThresholdPublisher : public ISubscriber<double> {
           public:
            explicit ProxyThresholdPublisher(CameraConfiguration* pCfg)
                : cfg(pCfg) {}

            void subscribe(ThresholdEventSubscriber* subscriber) {
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
        ProxyThresholdPublisher thresholdPublisher;

       protected:
        void ProcessFrame(TFrame& frame);

        void ChangeDetected();
    };

    template <typename TFrame>
    CameraObserver<TFrame>::CameraObserver(CameraConfiguration* pCfg)
        : cfg(pCfg),
          frameProcessor(this->cfg->roi, this->cfg->noiseThreshold,
                         this->cfg->rotation),
          thresholdManager(this->cfg->minimumChangeThreshold,
                           this->cfg->increaseThresholdFactor,
                           this->cfg->increaseThresholdFactor),
          thresholdPublisher(cfg) {
        this->running = false;
        this->waitingBufferFill = false;

        // VideoBuffer needs to be initialized this way since
        // we only know the buffer size here
        const bool szbuffer = this->cfg->videoValidatorBufferSize / 2;
        this->validator =
            std::make_unique<VideoBuffer<TFrame>>(szbuffer, szbuffer);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::Start() {
        TFrame frame;

        const auto minTimeBetweenFrames = 1000 / this->cfg->fps;

        timerFrames.Start();

        while (this->running) {
            if (this->source.GetNextFrame(frame)) {
                auto duration = timerFrames.GetDurationAndRestart();

                if (duration >= minTimeBetweenFrames) {
                    this->framePublisher.notifySubscribers(
                        this->cfg->positionOnOutput, frame);
                    this->ProcessFrame(frame);
                }
            }
        }
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::ProcessFrame(TFrame& frame) {
        // get change from the last frame
        double change =
            this->frameProcessor.NormalizeFrame(frame).DetectChanges();

        // get the average change
        double average = this->thresholdManager.GetAverage();

        if (change > average) {
            this->ChangeDetected();
        }

        // buffer is full, and we were waiting to Fill the buffer
        if (this->validator->AddFrame(frame) && this->waitingBufferFill) {
            this->waitingBufferFill = false;
            auto event = this->validator->GetEventFound();
            this->cameraEventsPublisher.notifySubscribers(this->cfg,
                                                          std::move(event));
        }

        // give the change found to the thresh manager
        this->thresholdManager.Add(change);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::ChangeDetected() {
        this->validator->ChangeWasDetected();
        this->waitingBufferFill = true;
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::SubscribeToCameraEvents(
        CameraEventSubscriber<TFrame>* subscriber) {
        this->cameraEventsPublisher.subscribe(subscriber);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::SubscribeToFramesUpdate(
        FrameEventSubscriber<TFrame>* subscriber) {
        this->framePublisher.subscribe(subscriber);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::SubscribeToThresholdUpdate(
        ThresholdEventSubscriber* subscriber) {
        this->thresholdPublisher.subscribe(subscriber);
    }

}  // namespace Observer
