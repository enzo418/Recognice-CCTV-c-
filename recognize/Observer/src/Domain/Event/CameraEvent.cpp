#include "CameraEvent.hpp"

namespace Observer {

    Frame& CameraEvent::GetFrameAt(int index) & { return this->frames[index]; }

    std::vector<Frame>& CameraEvent::GetFrames() & { return this->frames; }

    std::vector<Frame> CameraEvent::PopFrames() {
        return std::move(this->frames);
    }

    Size CameraEvent::GetFramesSize() { return this->framesSize; }

    double CameraEvent::GetFrameRate() { return this->frameRate; }

    void CameraEvent::SetFrameRate(double pFrameRate) {
        this->frameRate = pFrameRate;
    }

    void CameraEvent::SetFrameSize(Size pFrameSize) {
        this->framesSize = pFrameSize;
    }

}  // namespace Observer