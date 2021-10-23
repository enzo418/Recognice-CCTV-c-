#include "CameraObserver.hpp"

namespace Observer
{
    CameraObserver::CameraObserver(CameraConfiguration* pConfiguration) {
        this->cfg = pConfiguration;

        const bool szbuffer = this->cfg->videoValidatorBufferSize / 2;

        this->validator = std::make_unique<VideoValidator>(szbuffer, szbuffer);
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
        double avrg = this->thresholdManager.GetAverage();

        if (change > avrg) {
            this->ChangeDetected();
            this->thresholdPublisher.notifySubscribers(this->cfg, change);
        }

        // give the change found to the thresh manager
        this->thresholdManager.Add(change);
    }

    void CameraObserver::ChangeDetected() {
        auto frames = this->validator->GetFrames();
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
