#include "CameraObserver.hpp"

namespace Observer
{
    CameraObserver::CameraObserver(CameraConfiguration* pCfg)
    :   cfg(pCfg),
        frameProcessor(this->cfg->roi, this->cfg->noiseThreshold),
        thresholdManager(this->cfg->minimumChangeThreshold, this->cfg->increaseThresholdFactor, this->cfg->increaseThresholdFactor),
        thresholdPublisher(cfg)
    {
        this->running = false;
        this->waitingBufferFill = false;

        // VideoBuffer needs to be initialized this way since
        // we only know the buffer size here
        const bool szbuffer = this->cfg->videoValidatorBufferSize / 2;
        this->validator = std::make_unique<VideoBuffer>(szbuffer, szbuffer);
    }

    void CameraObserver::Start() {
        cv::Mat frame;

        const auto minTimeBetweenFrames = 1000 / this->cfg->fps;
        
        timerFrames.Start();

        while (this->running)
        {
            if (this->source.GetNextFrame(frame))
            {
                auto duration = timerFrames.GetDurationAndRestart();

                if (duration >= minTimeBetweenFrames)
                {
                    this->framePublisher.notifySubscribers(this->cfg->positionOnOutput, frame);
                    this->ProcessFrame(frame);
                }
            }
        }
    }

    void CameraObserver::Stop() {
        this->running = false;
    }

    void CameraObserver::ProcessFrame(cv::Mat& frame) {
        // get change from the last frame
        double change = this->frameProcessor
                                    .NormalizeFrame(frame)
                                    .DetectChanges();

        // get the average change
        double average = this->thresholdManager.GetAverage();

        if (change > average) {
            this->ChangeDetected();
        }

        // buffer is full, and we were waiting to Fill the buffer
        if (this->validator->AddFrame(frame) && this->waitingBufferFill) {
            this->waitingBufferFill = false;
            auto event = this->validator->GetEventFound();
            this->cameraEventsPublisher.notifySubscribers(this->cfg, std::move(event));
        }

        // give the change found to the thresh manager
        this->thresholdManager.Add(change);
    }

    void CameraObserver::ChangeDetected() {
        this->validator->ChangeWasDetected();
        this->waitingBufferFill = true;
    }

    void CameraObserver::SubscribeToCameraEvents(CameraEventSubscriber* subscriber) {
        this->cameraEventsPublisher.subscribe(subscriber);
    }

    void CameraObserver::SubscribeToFramesUpdate(FrameEventSubscriber* subscriber) {
        this->framePublisher.subscribe(subscriber);
    }

    void CameraObserver::SubscribeToThresholdUpdate(ThresholdEventSubscriber* subscriber) {
        this->thresholdPublisher.subscribe(subscriber);
    }

} // namespace Observer
