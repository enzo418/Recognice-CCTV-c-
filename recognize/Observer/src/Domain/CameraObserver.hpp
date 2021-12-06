#pragma once

#include "../Log/log.hpp"
#include "../Pattern/Camera/IThresholdEventSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Timer.hpp"
#include "Configuration/CameraConfiguration.hpp"
#include "FrameProcessor.hpp"
#include "ThresholdManager.hpp"
#include "VideoBuffer.hpp"
#include "VideoSource.hpp"
#include "VideoWriter.hpp"

// CameraEventSubscriber
#include "../IFunctionality.hpp"
#include "FrameDisplay.hpp"
#include "NotificationsController.hpp"
#include "VideoSource.hpp"
#include "VideoWriter.hpp"

// FrameEventSubscriber
#include <chrono>
#include <iostream>
#include <string>

// std::optional
#include <optional>

// std::unique_ptr
#include <memory>

namespace Observer {
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

        void SubscribeToCameraEvents(
            ICameraEventSubscriber<TFrame>* subscriber);
        void SubscribeToFramesUpdate(IFrameSubscriber<TFrame>* subscriber);
        void SubscribeToThresholdUpdate(IThresholdEventSubscriber* subscriber);

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

        std::unique_ptr<VideoBuffer<TFrame>> videoBufferForValidation;

        // timer to get a new frame every x ms
        Timer<std::chrono::milliseconds> timerFrames;

        FrameProcessor<TFrame> frameProcessor;

        ThresholdManager thresholdManager;

        Publisher<int, TFrame> framePublisher;

        Publisher<CameraConfiguration*, CameraEvent<TFrame>>
            cameraEventsPublisher;

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

       protected:
        void ProcessFrame(TFrame& frame);

        void ChangeDetected();

        void NewVideoBuffer();

       private:
        void UpdateFPS();
        double averageFPS {0};
        int frameCount {0};
        Timer<std::chrono::seconds> timerFPS;
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

        this->NewVideoBuffer();
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::Start() {
        this->running = true;

        TFrame frame;

        const auto minTimeBetweenFrames = 1000 / this->cfg->fps;

        this->source.Open(this->cfg->url);

        if (!this->source.isOpened()) {
            OBSERVER_WARN("Couldn't connect to camera {}", this->cfg->url);
        }

        timerFrames.Start();
        timerFPS.Start();

        while (this->running) {
            if (this->source.GetNextFrame(frame)) {
                auto duration = timerFrames.GetDurationAndRestart();

                if (duration >= minTimeBetweenFrames) {
                    this->framePublisher.notifySubscribers(
                        this->cfg->positionOnOutput, frame);
                    this->ProcessFrame(frame);
                    this->UpdateFPS();
                }
            }
        }

        OBSERVER_TRACE("Camera '{}' closed", this->cfg->name);

        this->source.Close();
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::ProcessFrame(TFrame& frame) {
        // buffer ready means that both sub-buffer have been filled
        if (this->videoBufferForValidation->AddFrame(frame) ==
            BufferState::BUFFER_READY) {
            auto event = this->videoBufferForValidation->GetEventFound();
            event.SetFrameRate(this->averageFPS);

            OBSERVER_TRACE(
                "Change detected on camera '{}', notifying subscribers",
                this->cfg->name);

            this->cameraEventsPublisher.notifySubscribers(this->cfg,
                                                          std::move(event));

            this->NewVideoBuffer();
        }

        // get change from the last frame
        double change =
            this->frameProcessor.NormalizeFrame(frame).DetectChanges();

        // get the average change
        double average = this->thresholdManager.GetAverage();

        if (change > average) {
            OBSERVER_TRACE("Change {} - AVRG: {}", change, average);
            this->ChangeDetected();
        }

        // give the change found to the thresh manager
        this->thresholdManager.Add(change);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::ChangeDetected() {
        this->videoBufferForValidation->ChangeWasDetected();
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::SubscribeToCameraEvents(
        ICameraEventSubscriber<TFrame>* subscriber) {
        this->cameraEventsPublisher.subscribe(subscriber);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::SubscribeToFramesUpdate(
        IFrameSubscriber<TFrame>* subscriber) {
        this->framePublisher.subscribe(subscriber);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        this->thresholdPublisher.subscribe(subscriber);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::NewVideoBuffer() {
        const auto szbuffer = this->cfg->videoValidatorBufferSize / 2;
        this->videoBufferForValidation =
            std::make_unique<VideoBuffer<TFrame>>(szbuffer, szbuffer);
    }

    template <typename TFrame>
    void CameraObserver<TFrame>::UpdateFPS() {
        frameCount++;

        if (timerFPS.GetDuration() >= 1) {
            this->averageFPS = (this->averageFPS + frameCount) / 2;

            // restart timer
            timerFPS.Start();

            this->frameCount = 0;

            OBSERVER_TRACE("FPS: {}", this->averageFPS);
        }
    }
}  // namespace Observer
