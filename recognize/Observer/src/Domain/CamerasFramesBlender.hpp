#pragma once

#include <algorithm>
#include <atomic>
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

       protected:
        void CalculateSleepTime(int totalNow);

       private:
        std::vector<std::queue<TFrame>> frames;
        std::mutex mtxFrames;

        OutputPreviewConfiguration* cfg;

        int maxFrames;

        bool running;

       private:
        int sleepForMs;

       private:
        Publisher<TFrame> framePublisher;
        std::atomic<int> frameSubscriberCount;
    };

    template <typename TFrame>
    CamerasFramesBlender<TFrame>::CamerasFramesBlender(
        OutputPreviewConfiguration* pCfg)
        : cfg(pCfg) {
        this->running = false;
        this->maxFrames = -1;
        this->sleepForMs = 50;
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

            int totalNumberFrames = 0;
            for (int i = 0; i < this->maxFrames; i++) {
                totalNumberFrames += frames[i].size();

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

            this->CalculateSleepTime(totalNumberFrames);

            std::this_thread::sleep_for(std::chrono::milliseconds(sleepForMs));
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

    template <typename TFrame>
    void CamerasFramesBlender<TFrame>::CalculateSleepTime(int totalFrames) {
        /**
         * if we have 4 cameras and totalFrames is 4, all the
         * cameras have at least 1 frame, so we are ok with the sleep timing
         * sleep maintains the same.
         *
         * if we have 4 cameras and totalFrames is 0, not even one cameras has a
         * frame available, we are much ahead of time. rest is < 0, sleep time
         * is increased.
         *
         * if we have 4 cameras and totalFrames is 8, we are behind. 8 - 4 = 4
         * rest > 0, sleep is decreased.
         */

        // increase/decrease by 5 ms
        constexpr int maxStep = 50;

        // max sleep time = 1 second
        constexpr int maxSleepTime = 1 * 1000;

        int rest =
            std::min(std::max(totalFrames - maxFrames, -maxStep), maxStep);

        sleepForMs -= rest;

        // can't be < 0 & > max
        sleepForMs = std::min(std::max(sleepForMs, 0), maxSleepTime);
    }
}  // namespace Observer
