#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../IFunctionality.hpp"
#include "../ImageDisplay.hpp"
#include "../ImageTransformation.hpp"
#include "../Log/log.hpp"
#include "../Pattern/Camera/IFrameSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Semaphore.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "Configuration/Configuration.hpp"
#include "Configuration/OutputPreviewConfiguration.hpp"

namespace Observer {
    /**
     * @todo write docs
     */
    template <typename TFrame>
    class FrameDisplay : public ISubscriber<TFrame>, public IFunctionality {
       public:
        /**
         * @param total Number of frames to display at the same time
         */
        explicit FrameDisplay();

        void SetNumberCameras(int total);

        void Start() override;

        void Stop() override;

        void update(TFrame frame) override;

       private:
        TFrame frame;
        std::mutex mutexFrame;

        bool running;
    };

    template <typename TFrame>
    FrameDisplay<TFrame>::FrameDisplay() {
        this->running = false;
        this->frame = ImageTransformation<TFrame>::BlackImage();
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::Start() {
        this->running = true;

        ImageDisplay<TFrame>::CreateWindow("images");

        while (this->running) {
            mutexFrame.lock();
            ImageDisplay<TFrame>::ShowImage("images", this->frame);
            mutexFrame.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        ImageDisplay<TFrame>::DestroyWindow("images");
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::update(TFrame pFrame) {
        std::lock_guard<std::mutex> lck(mutexFrame);

        ImageTransformation<TFrame>::CopyImage(pFrame, this->frame);
    }
}  // namespace Observer
