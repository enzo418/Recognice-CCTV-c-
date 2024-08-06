#include "CameraEvent.hpp"

#include "observer/Log/log.hpp"

namespace Observer {

    Frame& CameraEvent::GetFrameAt(int index) & { return this->frames[index]; }

    std::vector<Frame>& CameraEvent::GetFrames() & { return this->frames; }

    std::vector<Frame> CameraEvent::PopFrames() {
        return std::move(this->frames);
    }

    Size CameraEvent::GetFramesSize() const { return this->framesSize; }

    double CameraEvent::GetFrameRate() const { return this->frameRate; }

    int CameraEvent::GetGroupID() const {
        OBSERVER_ASSERT(this->groupID != -1, "Logic error: group id not set.");
        return this->groupID;
    }

    void CameraEvent::SetGroupID(int gID) { this->groupID = gID; }

    void CameraEvent::SetFrameRate(double pFrameRate) {
        this->frameRate = pFrameRate;
    }

    void CameraEvent::SetFrameSize(Size pFrameSize) {
        this->framesSize = pFrameSize;
    }

    void CameraEvent::SetOriginalFrameSize(Size pFrameSize) {
        this->originalFrameSize = pFrameSize;
    }

    Size CameraEvent::GetOriginalFrameSize() const {
        return this->originalFrameSize;
    }
}  // namespace Observer