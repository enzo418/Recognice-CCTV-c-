#include "SourceOnDemand.hpp"

#include "observer/IFrame.hpp"
#include "observer/Size.hpp"

namespace Web::Streaming::Video {

    SourceOnDemand::SourceOnDemand(int quality) : quality(quality) {
        firstFrameWasEncoded.clear();
        latestFrameWasEncoded.clear();

        timerSinceLastRequest.Start();

        this->status = LiveViewStatus::CLOSED;

        auto frame = Observer::Frame(Observer::Size(640, 360), 3);
        this->SetFrame(frame);
        this->EnsureEncoded();
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
        if (!Observer::has_flag(this->status, LiveViewStatus::OPEN)) {
            OBSERVER_WARN("Source is not yet open");
        }

        timerSinceLastRequest.Restart();

        EnsureEncoded();

        if (firstFrameWasEncoded.test()) {
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

    void SourceOnDemand::SetFrame(Observer::Frame newFrame) {
        std::lock_guard<std::mutex> lock(mtxFrame);
        newFrame.CopyTo(frame);
        latestFrameWasEncoded.clear();
    }

    void SourceOnDemand::EnsureEncoded() {
        std::lock_guard<std::mutex> lock(mtxFrame);
        if (!latestFrameWasEncoded.test() && !frame.IsEmpty()) {
            std::lock_guard<std::mutex> lock(mtxLatestImageBuffer);
            frame.EncodeImage(".jpg", quality, latestImageBuffer);
            latestFrameWasEncoded.test_and_set();
            firstFrameWasEncoded.test_and_set();
        }
    }

    void SourceOnDemand::EnsureOpenAndReady() {
        // NOTE: Do not check for OPEN/CLOSED because if it a "Zombie" source,
        // it will be Stop()ed (!IsRunning) and then Start()ed again here.
        // Besides, that would make more complex the synchronization or cause
        // deadlocks.

        if (!IsRunning()) {
            OBSERVER_TRACE("Opening source");
            this->Start();
        } else if (!Observer::has_flag(this->status, LiveViewStatus::OPEN)) {
            OBSERVER_TRACE("The source is not yet open");
            this->EnsureOpen();
        }
    }

}  // namespace Web::Streaming::Video