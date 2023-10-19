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
                    "Live view '{}' was stopped because it had no clients "
                    "after {} seconds",
                    cameraUri, SECONDS_TO_ZOMBIE_ON_DEMAND);

                this->Stop();
                return;
            }

            if (source.IsFrameAvailable()) {
                this->SetFrame(std::move(source.GetFrame()));
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
        opening.test_and_set();

        if (source.TryOpen(cameraUri)) {
            OBSERVER_TRACE("Camera source opened: {}", cameraUri);
            Observer::clear_flag(this->status, LiveViewStatus::CLOSED);
            Observer::clear_flag(this->status, LiveViewStatus::ERROR);

            Observer::set_flag(this->status, LiveViewStatus::OPEN);
        } else {
            OBSERVER_ERROR("Camera source failed to open: {}", cameraUri);
            Observer::set_flag(this->status, LiveViewStatus::ERROR);
        }

        opening.clear();
    }

    void CameraOnDemand::EnsureOpen() {
        if (!opening.test() &&
            !Observer::has_flag(this->status, LiveViewStatus::OPEN)) {
            OBSERVER_TRACE("Opening camera source: {}", cameraUri);
            this->OpenCamera();
        } else {
            OBSERVER_TRACE("Source was running: {}", cameraUri);
        }
    }

}  // namespace Web::Streaming::Video