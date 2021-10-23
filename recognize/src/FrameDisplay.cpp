#include "FrameDisplay.hpp"

namespace Observer {
    void FrameDisplay::Start() {
        // TODO: Show frames
    }

    void FrameDisplay::update(int cameraPos, cv::Mat frame) {
        // TODO: Lock vector
        this->frames[cameraPos] = frame;
    }
}
