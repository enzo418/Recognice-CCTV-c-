#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../IFunctionality.hpp"
#include "../ImageDisplay.hpp"
#include "../ImageTransformation.hpp"
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
    class FrameDisplay : public IFrameSubscriber<TFrame>,
                         public IFunctionality {
       public:
        /**
         * @param total Number of frames to display at the same time
         */
        explicit FrameDisplay(OutputPreviewConfiguration* cfg);

        void SetNumberCameras(int total);

        void Start() override;

        void Stop() override;

        /**
         * @brief add a new frame to the available frames
         *
         * @param framePosition 0 = top left, 1 = top right, ...
         */
        void update(int framePosition, TFrame frame) override;

       private:
        std::vector<std::queue<TFrame>> frames;
        std::mutex mtxFrames;

        OutputPreviewConfiguration* cfg;

        int maxFrames;

        bool running;
    };

    template <typename TFrame>
    FrameDisplay<TFrame>::FrameDisplay(OutputPreviewConfiguration* pCfg)
        : cfg(pCfg) {
        this->running = false;
        this->maxFrames = -1;
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::Start() {
        OBSERVER_ASSERT(
            this->maxFrames != -1,
            "SetNumberCameras should be called before calling start");

        this->running = true;

        ImageDisplay<TFrame>::CreateWindow("images");

        TFrame frame;

        std::vector<TFrame> framesToShow(maxFrames);
        std::vector<bool> cameraFirstFrameReaded(maxFrames);

        const auto maxHStack = this->maxFrames == 1 ? 1 : 2;

        TFrame* referenceFrameForBlankImage = nullptr;

        while (this->running) {
            this->mtxFrames.lock();

            for (int i = 0; i < this->maxFrames; i++) {
                if (!frames[i].empty()) {
                    cameraFirstFrameReaded[i] = true;
                    framesToShow[i] = this->frames[i].front();
                    referenceFrameForBlankImage = &framesToShow[i];
                    this->frames[i].pop();
                } else if (!cameraFirstFrameReaded[i]) {
                    framesToShow[i] = ImageTransformation<TFrame>::BlackImage(
                        referenceFrameForBlankImage);
                }
            }

            this->mtxFrames.unlock();

            frame = ImageTransformation<TFrame>::StackImages(
                &framesToShow[0], this->maxFrames, maxHStack);

            if (!this->cfg->resolution.empty()) {
                ImageTransformation<TFrame>::Resize(frame, frame,
                                                    this->cfg->resolution);
            } else {
                ImageTransformation<TFrame>::Resize(frame, frame,
                                                    this->cfg->scaleFactor,
                                                    this->cfg->scaleFactor);
            }

            ImageDisplay<TFrame>::ShowImage("images", frame);

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        ImageDisplay<TFrame>::DestroyWindow("images");
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::update(int cameraPos, TFrame frame) {
        this->mtxFrames.lock();
        this->frames[cameraPos].push(frame);
        this->mtxFrames.unlock();
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::SetNumberCameras(int total) {
        this->frames.resize(total);
        this->maxFrames = total;
    }
}  // namespace Observer
