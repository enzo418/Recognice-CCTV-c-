#include "CameraObserver.hpp"

namespace Observer
{
    CameraObserver::CameraObserver(const std::string& url, CameraConfiguration configuration) {
        // the compiler should be able to optimize it moving the configuration into the cfg stack
        this->cfg.emplace(configuration);

        const bool szbuffer = this->cfg->videoValidatorBufferSize / 2;

        this->validator = std::make_unique<VideoValidator>(szbuffer, szbuffer);
    }

    void CameraObserver::Start() {
        cv::Mat frame;
        Timer<std::chrono::milliseconds> timerFrames(true);
        const auto minTimeBetweenFrames = 1000 / this->cfg->fps;

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
        
    }

} // namespace Observer
