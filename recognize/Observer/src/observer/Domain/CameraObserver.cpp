#include "CameraObserver.hpp"

#include <chrono>

namespace Observer {
    CameraObserver::CameraObserver(CameraConfiguration* pCfg) : cfg(pCfg) {
        this->running = false;
    }

    void CameraObserver::InternalStart() {
        this->running = true;

        const bool processFrames = cfg->type != ECameraType::VIEW;

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

    int CameraObserver::GetFPS() { return this->fps; }

    std::string CameraObserver::GetName() { return this->cfg->name; }
}  // namespace Observer
