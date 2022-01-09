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
     * @todo Consumes frames from the camera, combines them into a single image
     * and notifies to all its subscribers.
     */
    template <typename TFrame>
    class CamerasFramesBlender : public IFrameSubscriber<TFrame>,
                                 public IFunctionality {
       public:
        /**
         * @param total Number of frames to display at the same time
         */
        explicit CamerasFramesBlender(OutputPreviewConfiguration* cfg);

        void SetNumberCameras(int total);

        void Start() override;

        void Stop() override;

        /**
         * @brief add a new frame to the available frames
         *
         * @param framePosition 0 = top left, 1 = top right, ...
         */
        void update(int framePosition, TFrame frame) override;

        void SubscribeToFramesUpdate(ISubscriber<TFrame>* sub);

       private:
        std::vector<std::queue<TFrame>> frames;
        std::mutex mtxFrames;

        OutputPreviewConfiguration* cfg;

        int maxFrames;

        bool running;

       private:
        Publisher<TFrame> framePublisher;
        int frameSubscriberCount;
    };

    template <typename TFrame>
    CamerasFramesBlender<TFrame>::CamerasFramesBlender(
        OutputPreviewConfiguration* pCfg)
        : cfg(pCfg) {
        this->running = false;
        this->maxFrames = -1;
    }

    template <typename TFrame>
    void CamerasFramesBlender<TFrame>::Start() {
        OBSERVER_ASSERT(
            this->maxFrames != -1,
            "SetNumberCameras should be called before calling start");

        this->running = true;

        std::vector<TFrame> framesToShow(maxFrames);
        std::vector<bool> cameraFirstFrameReaded(maxFrames);

        const auto maxHStack = this->maxFrames == 1 ? 1 : 2;

        TFrame* referenceFrameForBlankImage = nullptr;

        while (this->running) {
            this->mtxFrames.lock();

            for (int i = 0; i < this->maxFrames; i++) {
                /// TODO: .empty is an opencv function...
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

            TFrame frame = ImageTransformation<TFrame>::StackImages(
                &framesToShow[0], this->maxFrames, maxHStack);

            if (!this->cfg->resolution.empty()) {
                ImageTransformation<TFrame>::Resize(frame, frame,
                                                    this->cfg->resolution);
            } else {
                ImageTransformation<TFrame>::Resize(frame, frame,
                                                    this->cfg->scaleFactor,
                                                    this->cfg->scaleFactor);
            }

            framePublisher.notifySubscribers(std::move(frame));

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    template <typename TFrame>
    void CamerasFramesBlender<TFrame>::Stop() {
        this->running = false;
    }

    template <typename TFrame>
    void CamerasFramesBlender<TFrame>::update(int cameraPos, TFrame frame) {
        if (frameSubscriberCount > 0) {
            this->mtxFrames.lock();
            this->frames[cameraPos].push(frame);
            this->mtxFrames.unlock();
        }
    }

    template <typename TFrame>
    void CamerasFramesBlender<TFrame>::SetNumberCameras(int total) {
        this->frames.resize(total);
        this->maxFrames = total;
    }

    template <typename TFrame>
    void CamerasFramesBlender<TFrame>::SubscribeToFramesUpdate(
        ISubscriber<TFrame>* sub) {
        framePublisher.subscribe(sub);
        frameSubscriberCount++;
    }
}  // namespace Observer
