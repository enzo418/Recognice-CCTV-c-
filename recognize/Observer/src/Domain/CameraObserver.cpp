#include "CameraObserver.hpp"

namespace Observer {
    CameraObserver::CameraObserver(CameraConfiguration* pCfg)
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

    void CameraObserver::InternalStart() {
        OBSERVER_ASSERT(cfg->fps != 0, "FPS cannot be 0.");

        this->running = true;

        const bool processFrames = cfg->type != ECameraType::VIEW;

        // Try to open the camera
        if (!this->source.TryOpen(this->cfg->url)) {
            OBSERVER_WARN("Couldn't connect to camera {}", this->cfg->url);
            this->source.Close();
            return;
        }

        const int fps = source.GetFPS();
        OBSERVER_INFO("FPS: {}", fps);

        // number of ms that the cameras waits to send a new frame
        const double realTimeBetweenFrames = 1000.0 / fps;

        // this is the milliseconds expected by the user betweem two frames
        const auto minTimeBetweenFrames = 1000.0 / this->cfg->fps;

        // Timer to only process frames between some time
        Timer<std::chrono::milliseconds> timerFakeFPS(true);

        // Timer to not waste cpu time
        Timer<std::chrono::milliseconds> timerRealFPS(true);

        // wait accumulative stores the duration of each frame sent and
        // processed, if it's < 0 we are ahead so we need to sleep the thread to
        // save cpu work, else we took to much time processing the frames so no
        // sleep is required.
        int waitAccumulative = 0;

        Frame frame;
        while (this->running && source.IsOk()) {
            if (source.IsFrameAvailable()) {
                frame = source.GetFrame();
                if (!frame.IsEmpty()) {
                    this->framePublisher.notifySubscribers(
                        this->cfg->positionOnOutput, frame);

                    if (processFrames &&
                        timerFakeFPS.GetDuration() > minTimeBetweenFrames) {
                        this->ProcessFrame(frame);

                        timerFakeFPS.Restart();
                    }
                }
            }

            auto duration = timerRealFPS.GetDuration();

            int sleepExactly = realTimeBetweenFrames - duration;

            int buffered = source.BufferedAmmount();
            if (sleepExactly > 0 && buffered == 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sleepExactly));
            }

            timerRealFPS.Restart();
        }

        OBSERVER_TRACE("Camera '{}' closed", this->cfg->name);

        this->source.Close();
    }

    void CameraObserver::ProcessFrame(Frame& frame) {
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

    void CameraObserver::ChangeDetected() {
        this->videoBufferForValidation->ChangeWasDetected();
    }

    void CameraObserver::SubscribeToCameraEvents(
        ICameraEventSubscriber* subscriber) {
        this->cameraEventsPublisher.subscribe(subscriber);
    }

    void CameraObserver::SubscribeToFramesUpdate(IFrameSubscriber* subscriber) {
        this->framePublisher.subscribe(subscriber);
    }

    void CameraObserver::SubscribeToThresholdUpdate(
        IThresholdEventSubscriber* subscriber) {
        this->thresholdPublisher.subscribe(subscriber);
    }

    void CameraObserver::NewVideoBuffer() {
        const auto szbuffer = this->cfg->videoValidatorBufferSize / 2;
        this->videoBufferForValidation =
            std::make_unique<VideoBuffer>(szbuffer, szbuffer);
    }
}  // namespace Observer
