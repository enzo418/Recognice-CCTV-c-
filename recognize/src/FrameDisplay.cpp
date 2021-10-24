#include "FrameDisplay.hpp"
#include "SimpleBlockingQueue.hpp"

namespace Observer {
    FrameDisplay::FrameDisplay(int total) : maxFrames(total) {
        this->running = false;
        this->frames.reserve(total);
    }

    void FrameDisplay::Start() {
        this->running = true;

        std::vector<cv::Mat> framesToShow(maxFrames);
        while(this->running) {
            for(int i = 0; i < this->maxFrames; i++) {
                framesToShow[i] = this->frames[i].pop();
            }

            // TODO: Show frames

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void FrameDisplay::Stop() {
        this->running = false;
    }

    void FrameDisplay::update(int cameraPos, cv::Mat frame) {
        this->frames[cameraPos].push(frame);
    }
}
