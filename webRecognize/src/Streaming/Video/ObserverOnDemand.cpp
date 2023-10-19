#include "ObserverOnDemand.hpp"

namespace Web::Streaming::Video {

    ObserverOnDemand::ObserverOnDemand(int quality) : SourceOnDemand(quality) {}

    void ObserverOnDemand::InternalStart() {
        this->status = LiveViewStatus::OPEN | LiveViewStatus::RUNNING;
    }

    void ObserverOnDemand::PostStop() {
        this->status = LiveViewStatus::CLOSED | LiveViewStatus::STOPPED;
    }

    void ObserverOnDemand::update(Observer::Frame frame) {
        this->SetFrame(frame);
    }

    void ObserverOnDemand::EnsureOpen() {
        // it doesn't matter.
        if (!Observer::has_flag(this->status, LiveViewStatus::OPEN)) {
            this->status = LiveViewStatus::OPEN | LiveViewStatus::RUNNING;
        }
    }
}  // namespace Web::Streaming::Video