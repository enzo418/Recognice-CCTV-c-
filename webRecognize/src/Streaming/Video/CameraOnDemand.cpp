#include "CameraOnDemand.hpp"

#include "Utils/ThreadName.hpp"
#include "observer/Utils/SpecialEnums.hpp"

namespace Web::Streaming::Video {

    CameraOnDemand::CameraOnDemand(const std::string& pCameraUri, int quality)
        : SourceOnDemand(quality) {
        this->cameraUri = pCameraUri;
    }

    void CameraOnDemand::InternalStart() {
        SetThreadName("CameraOnDemand");

        if (!Observer::has_flag(this->status, LiveViewStatus::OPEN) ||
            Observer::has_flag(this->status, LiveViewStatus::STOPPED)) {
            OBSERVER_TRACE("Opening camera source: {}", cameraUri);
            this->OpenCamera();
        } else {
            OBSERVER_TRACE("Source was running: {}", cameraUri);
        }

        this->source.Start();

        if (Observer::has_flag(status, LiveViewStatus::STOPPED)) {
            Observer::clear_flag(status, LiveViewStatus::STOPPED);
        }

        Observer::set_flag(status, LiveViewStatus::RUNNING);

        while (this->running) {
            if (this->IsZombie()) {
                OBSERVER_TRACE(
                    "Live view was stopped because it had no clients after {} "
                    "seconds",
                    SECONDS_TO_ZOMBIE);

                this->Stop();
                return;
            }

            if (source.IsFrameAvailable()) {
                this->SetFrame(source.GetFrame());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds((int)15));
        }

        Observer::clear_flag(status, LiveViewStatus::RUNNING);
        Observer::set_flag(status, LiveViewStatus::STOPPED);
    }

    void CameraOnDemand::PostStop() {
        source.Stop();
        Observer::clear_flag(this->status, LiveViewStatus::OPEN);
    }

    void CameraOnDemand::OpenCamera() {
        if (source.TryOpen(cameraUri)) {
            Observer::clear_flag(this->status, LiveViewStatus::CLOSED);
            Observer::clear_flag(this->status, LiveViewStatus::ERROR);

            Observer::set_flag(this->status, LiveViewStatus::OPEN);
        } else {
            Observer::set_flag(this->status, LiveViewStatus::ERROR);
        }
    }

}  // namespace Web::Streaming::Video