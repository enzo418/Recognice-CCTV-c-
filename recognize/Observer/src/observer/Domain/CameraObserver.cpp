#include "CameraObserver.hpp"

#include <chrono>

namespace Observer {
    CameraObserver::CameraObserver(CameraConfiguration* pCfg)
        : cfg(pCfg), thresholdPublisher(cfg) {
        this->running = false;

        this->type = cfg->type;
    }

    void CameraObserver::InternalStart() {
        this->running = true;

        // Try to open the camera
        if (!this->source.TryOpen(this->cfg->url)) {
            OBSERVER_WARN("Couldn't connect to camera {}", this->cfg->url);
            this->source.Close();
            return;
        }

        this->source.Start();

        const int fps = source.GetFPS();
        OBSERVER_INFO("FPS: {}", fps);

        const double cfgFps = cfg->fps == 0 ? fps : cfg->fps;

        // number of ms that the cameras waits to send a new frame
        const double realTimeBetweenFrames = 1000.0 / fps;

        // this is the milliseconds expected by the user between two frames
        const auto minTimeBetweenFrames = 1000.0 / cfgFps;

        this->fps =
            1000.0 / std::max(realTimeBetweenFrames, minTimeBetweenFrames);

        // Timer to only process frames between some time
        Timer<std::chrono::milliseconds> timerFakeFPS(true);

        // Timer to not waste cpu time
        Timer<std::chrono::milliseconds> timerRealFPS(true);

        this->SetupDependencies();

        Frame frame;
        while (this->running && source.IsOk()) {
            const bool processFrames =
                this->type.load(std::memory_order_acquire) ==
                ECameraType::NOTIFICATOR;

            if (source.IsFrameAvailable()) {
                frame = source.GetFrame();
                if (!frame.IsEmpty()) {
                    this->framePublisher.notifySubscribers(
                        this->cfg->positionOnOutput, frame.Clone());

                    if (processFrames &&
                        timerFakeFPS.GetDuration() > minTimeBetweenFrames) {
                        this->ProcessFrame(frame);

                        timerFakeFPS.Restart();
                    }
                }
            }

            double duration = timerRealFPS.GetDuration();

            int sleepExactly = realTimeBetweenFrames - duration;

            int buffered = source.BufferedAmount();
            if (sleepExactly > 0 && buffered == 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(sleepExactly));
            }

            timerRealFPS.Restart();
        }

        OBSERVER_TRACE("Camera '{}' closed", this->cfg->name);

        this->source.Close();
    }

    void CameraObserver::SubscribeToFramesUpdate(IFrameSubscriber* subscriber) {
        this->framePublisher.subscribe(subscriber);
    }

    void CameraObserver::SetType(ECameraType t) {
        this->type.store(t, std::memory_order_release);

        if (t != ECameraType::DISABLED) {
            this->SetupDependencies();
        }
    }

    int CameraObserver::GetFPS() { return this->fps; }

    std::string CameraObserver::GetName() { return this->cfg->name; }

    ECameraType CameraObserver::GetType() { return this->type; }

    void CameraObserver::SetupDependencies() {
        if (this->type.load(std::memory_order_acquire) ==
            ECameraType::NOTIFICATOR) {
            if (!this->videoBufferForValidation ||
                (this->videoBufferForValidation &&
                 this->videoBufferForValidation->GetState() !=
                     BufferState::BUFFER_IDLE)) {
                // dismiss the current buffer
                this->NewVideoBuffer();
            }

            this->frameProcessor.Setup(this->source.GetInputResolution(),
                                       this->cfg->processingConfiguration,
                                       this->cfg->rotation);

            this->thresholdManager.Setup(
                this->cfg->minimumChangeThreshold,
                this->cfg->secondsBetweenThresholdUpdate,
                this->cfg->increaseThresholdFactor);
        }
    }

    /* ---------------------- MOVEMENT ---------------------- */

    void CameraObserver::ProcessFrame(Frame& frame) {
        if (!cfg->resizeTo.empty()) frame.Resize(cfg->resizeTo);

        // buffer ready means that both sub-buffer have been filled
        if (this->videoBufferForValidation->AddFrame(frame) ==
            BufferState::BUFFER_READY) {
            // get the frames that triggered the event
            auto frames = this->videoBufferForValidation->PopAllFrames();

            // build the event
            std::shared_ptr<CameraEvent> event = std::make_shared<CameraEvent>(
                std::move(frames),
                this->videoBufferForValidation->GetIndexMiddleFrame());

            event->SetFrameRate(this->GetFPS());
            event->SetFrameSize(frame.GetSize());

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
