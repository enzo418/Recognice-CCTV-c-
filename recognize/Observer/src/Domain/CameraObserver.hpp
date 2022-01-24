#pragma once

#include "../Log/log.hpp"
#include "../Pattern/Camera/IThresholdEventSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Timer.hpp"
#include "BufferedSource.hpp"
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
#include <thread>

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
        BufferedSource<TFrame> source;

        // video output
        VideoWriter<TFrame> writer;

        std::unique_ptr<VideoBuffer<TFrame>> videoBufferForValidation;

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
    };

    template <typename TFrame>
    CameraObserver<TFrame>::CameraObserver(CameraConfiguration* pCfg)
        : cfg(pCfg),
          frameProcessor(this->cfg->processingConfiguration.resize,
                         this->cfg->processingConfiguration.roi,
                         this->cfg->processingConfiguration.noiseThreshold,
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
        OBSERVER_ASSERT(cfg->fps != 0, "FPS cannot be 0.");

        this->running = true;

        const bool processFrames = cfg->type != ECameraType::VIEW;

        // Try to open the camera
        if (!this->source.TryOpen(this->cfg->url)) {
            OBSERVER_WARN("Couldn't connect to camera {}", this->cfg->url);
            this->source.Close();
            return;
        }

        // this is the milliseconds expected by the user betweem two frames
        const auto minTimeBetweenFrames = 1000.0 / this->cfg->fps;

        // Timer to only process frames between some time
        Timer<std::chrono::milliseconds> timerFakeFPS(true);

        // wait accumulative stores the duration of each frame sent and
        // processed, if it's < 0 we are ahead so we need to sleep the thread to
        // save cpu work, else we took to much time processing the frames so no
        // sleep is required.
        int waitAccumulative = 0;

        while (this->running) {
            if (source.IsOk() && source.IsFrameAvailable()) {
                TFrame frame = source.GetFrame();

                this->framePublisher.notifySubscribers(
                    this->cfg->positionOnOutput, frame);

                if (processFrames &&
                    timerFakeFPS.GetDuration() > minTimeBetweenFrames) {
                    this->ProcessFrame(frame);

                    timerFakeFPS.Restart();
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
        ImageTransformation<TFrame>::Resize(frame, frame, cfg->resizeTo);

        // buffer ready means that both sub-buffer have been filled
        if (this->videoBufferForValidation->AddFrame(frame) ==
            BufferState::BUFFER_READY) {
            // get the frames that triggered the event
            auto frames = this->videoBufferForValidation->PopAllFrames();

            // build the event
            CameraEvent event(
                std::move(frames),
                this->videoBufferForValidation->GetIndexMiddleFrame());

            event.SetFrameRate(this->source.GetFPS());
            event.SetFrameSize(ImageTransformation<TFrame>::GetSize(frame));

            OBSERVER_TRACE(
                "Change detected on camera '{}', notifying subscribers",
                this->cfg->name);

            // notify our subscribers
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
}  // namespace Observer
