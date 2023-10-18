#include "SourceOnDemand.hpp"

#include "observer/IFrame.hpp"
#include "observer/Size.hpp"

namespace Web::Streaming::Video {

    SourceOnDemand::SourceOnDemand(int quality) : quality(quality) {
        firstFrameWasEncoded.reset();
        latestFrameWasEncoded.reset();

        this->status = LiveViewStatus::CLOSED;

        this->SetFrame(Observer::Frame(Observer::Size(640, 360), 3));
    }

    LiveViewStatus SourceOnDemand::GetStatus() { return status; }

    /**
     * @brief Run the callback on the frame. If there is no frame, it will
     * return false.
     *
     * @param callback The callback to run on the frame.
     * @return true The callback was run.
     * @return false
     */
    bool SourceOnDemand::RunSafelyOnFrameReady(EncodedImageCallback callback) {
        OBSERVER_ASSERT(Observer::has_flag(this->status, LiveViewStatus::OPEN),
                        "Source is not open");

        timerSinceLastRequest.Restart();

        EnsureEncoded();

        if (firstFrameWasEncoded) {
            std::lock_guard<std::mutex> lock(mtxLatestImageBuffer);
            callback(latestImageBuffer);

            return true;
        }

        return false;
    }

    bool SourceOnDemand::IsZombie() {
        return Observer::has_flag(this->status, LiveViewStatus::OPEN) &&
               !Observer::has_flag(this->status, LiveViewStatus::STOPPED) &&
               timerSinceLastRequest.GetDuration() >
                   SECONDS_TO_ZOMBIE_ON_DEMAND;
    }

    void SourceOnDemand::SetFrame(const Observer::Frame& newFrame) {
        std::lock_guard<std::mutex> lock(mtxFrame);
        frame = newFrame;
        latestFrameWasEncoded.reset();
    }

    void SourceOnDemand::EnsureEncoded() {
        std::lock_guard<std::mutex> lock(mtxFrame);
        if (!latestFrameWasEncoded && !frame.IsEmpty()) {
            frame.EncodeImage(".jpg", quality, latestImageBuffer);
            latestFrameWasEncoded.set();
            firstFrameWasEncoded.set();
        }
    }

    void SourceOnDemand::EnsureOpenAndReady() {
        if (!IsRunning()) {
            OBSERVER_TRACE("Opening source");
            this->Start();
        } else if (Observer::has_flag(this->status, LiveViewStatus::CLOSED)) {
            OBSERVER_TRACE("Source had an error");
            this->Stop();
            this->Start();
        } else {
            OBSERVER_TRACE("Source was running");
        }
    }

}  // namespace Web::Streaming::Video