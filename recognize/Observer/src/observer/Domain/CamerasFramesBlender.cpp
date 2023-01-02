#include "CamerasFramesBlender.hpp"

namespace Observer {

    CamerasFramesBlender::CamerasFramesBlender(OutputPreviewConfiguration* pCfg)
        : cfg(pCfg) {
        this->running = false;
        this->maxFrames = -1;
        this->sleepForMs = 50;
    }

    void CamerasFramesBlender::InternalStart() {
        OBSERVER_ASSERT(
            this->maxFrames != -1,
            "SetNumberCameras should be called before calling start");

        std::vector<Frame> framesToShow(maxFrames);
        std::vector<bool> cameraFirstFrameReaded(maxFrames);

        const auto maxHStack = this->maxFrames == 1 ? 1 : 2;

        static Frame referenceFrameForBlankImage(Size(640, 360), 3);

        while (this->running) {
            this->mtxFrames.lock();

            int totalNumberFrames = 0;
            for (int i = 0; i < this->maxFrames; i++) {
                totalNumberFrames += frames[i].size();

                /// TODO: .empty is an opencv function...
                if (!frames[i].empty()) {
                    cameraFirstFrameReaded[i] = true;
                    framesToShow[i] = this->frames[i].front();
                    referenceFrameForBlankImage = framesToShow[i];
                    this->frames[i].pop();
                } else if (!cameraFirstFrameReaded[i]) {
                    framesToShow[i] =
                        referenceFrameForBlankImage.GetBlackImage();
                }
            }

            this->mtxFrames.unlock();

            this->NormalizeNumberOfChannels(framesToShow);

            Frame frame = ImageDisplay::Get().StackImages(
                &framesToShow[0], this->maxFrames, maxHStack);

            if (!this->cfg->resolution.empty()) {
                frame.Resize(this->cfg->resolution);
            } else {
                frame.Resize(this->cfg->scaleFactor, this->cfg->scaleFactor);
            }

            framePublisher.notifySubscribers(std::move(frame));

            this->CalculateSleepTime(totalNumberFrames);

            std::this_thread::sleep_for(std::chrono::milliseconds(sleepForMs));
        }
    }

    void CamerasFramesBlender::update(int cameraPos, Frame frame) {
        if (frameSubscriberCount > 0) {
            this->mtxFrames.lock();
            this->frames[cameraPos].push(frame);
            this->mtxFrames.unlock();
        }
    }

    void CamerasFramesBlender::SetNumberCameras(int total) {
        this->frames.resize(total);
        this->maxFrames = total;
    }

    void CamerasFramesBlender::SubscribeToFramesUpdate(
        ISubscriber<Frame>* sub) {
        framePublisher.subscribe(sub);
        frameSubscriberCount++;
    }

    void CamerasFramesBlender::CalculateSleepTime(int totalFrames) {
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

    void CamerasFramesBlender::NormalizeNumberOfChannels(
        std::vector<Frame>& normalize) {
        // check if it's necessary to normalize
        for (auto& frame : normalize) {
            int channels = frame.GetNumberChannels();

            if (channels != 3) {
                frame.ToColorSpace(ColorSpaceConversion::COLOR_GRAY2RGB);
            }
        }
    }
}  // namespace Observer
