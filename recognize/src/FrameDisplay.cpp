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
            this->mtxFrames.lock();
            for(int i = 0; i < this->maxFrames; i++) {
                framesToShow[i] = this->frames[i].front();
                this->frames[i].pop();
            }
            this->mtxFrames.unlock();

            // TODO: Show frames

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void FrameDisplay::Stop() {
        this->running = false;
    }

    void FrameDisplay::update(int cameraPos, cv::Mat frame) {
        this->mtxFrames.lock();
        this->frames[cameraPos].push(frame);
        this->mtxFrames.unlock();
    }
}
