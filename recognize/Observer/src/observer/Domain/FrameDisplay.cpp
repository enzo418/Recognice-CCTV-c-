#include "FrameDisplay.hpp"

namespace Observer {
    FrameDisplay::FrameDisplay() : frame(Size(640, 360), 3) {
        this->running = false;
    }

    void FrameDisplay::InternalStart() {
        ImageDisplay::Get().CreateWindow("images");

        while (this->running) {
            mutexFrame.lock();
            ImageDisplay::Get().ShowImage("images", this->frame);
            mutexFrame.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        ImageDisplay::Get().DestroyWindow("images");
    }

    void FrameDisplay::update(Frame pFrame) {
        std::lock_guard<std::mutex> lck(mutexFrame);

        pFrame.CopyTo(this->frame);
    }
}  // namespace Observer
