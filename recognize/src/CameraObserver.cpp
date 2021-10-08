#include "CameraObserver.hpp"

namespace Observer
{
    CameraObserver::CameraObserver(CameraConfiguration configuration) {
        // the compiler should be able to optimize it moving the configuration into the cfg stack
        this->cfg.emplace(configuration);

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
        }

        // give the change found to the thresh manager
        this->thresholdManager.Add(change);
    }

    void CameraObserver::ChangeDetected() {

    }
} // namespace Observer
