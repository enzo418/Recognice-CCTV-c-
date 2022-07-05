#include "ActiveCamera.hpp"

namespace Observer {
    ActiveCamera::ActiveCamera(CameraConfiguration* pCfg)
        : CameraObserver(pCfg),
          frameProcessor(pCfg->processingConfiguration.resize,
                         pCfg->processingConfiguration.roi,
                         pCfg->processingConfiguration.noiseThreshold,
                         pCfg->rotation),
          thresholdManager(pCfg->minimumChangeThreshold,
                           pCfg->increaseThresholdFactor,
                           pCfg->increaseThresholdFactor),
          thresholdPublisher(pCfg) {
        this->NewVideoBuffer();
    }

    void ActiveCamera::ProcessFrame(Frame& frame) {
        frame.Resize(cfg->resizeTo);

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
            event.SetFrameSize(frame.GetSize());

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

    void ActiveCamera::ChangeDetected() {
        this->videoBufferForValidation->ChangeWasDetected();
    }

    void ActiveCamera::SubscribeToCameraEvents(
        ICameraEventSubscriber* subscriber) {
        this->cameraEventsPublisher.subscribe(subscriber);
    }

    void ActiveCamera::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        this->thresholdPublisher.subscribe(subscriber);
    }

    void ActiveCamera::NewVideoBuffer() {
        const auto szbuffer = this->cfg->videoValidatorBufferSize / 2;
        this->videoBufferForValidation =
            std::make_unique<VideoBuffer>(szbuffer, szbuffer);
    }
}  // namespace Observer