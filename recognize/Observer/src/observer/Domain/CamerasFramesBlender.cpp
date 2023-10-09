#include "CamerasFramesBlender.hpp"

#include "observer/CircularFIFO.hpp"

namespace Observer {

    CamerasFramesBlender::CamerasFramesBlender(OutputPreviewConfiguration* pCfg)
        : cfg(pCfg) {
        this->running = false;
        this->maxFrames = -1;
        this->sleepForMs = pCfg->maxOutputFps > 0
                               ? 1000 / pCfg->maxOutputFps
                               : /*defaults to 20 max fps*/ 1000 / 20;
    }

    void CamerasFramesBlender::InternalStart() {
        OBSERVER_ASSERT(
            this->maxFrames != -1,
            "SetNumberCameras should be called before calling start");

        OBSERVER_ASSERT(this->maxFrames > 0,
                        "SetNumberCameras should be called with a number > 0");

        std::vector<Frame> framesToShow(maxFrames);
        std::vector<bool> cameraFirstFrameRead(maxFrames);

        const auto maxHStack = this->maxFrames == 1 ? 1 : 2;

        static Frame referenceFrameForBlankImage(Size(640, 360), 3);

        while (this->running) {
            for (int i = 0; i < this->maxFrames; i++) {
                if (!frames[i]->empty()) {
                    cameraFirstFrameRead[i] = true;
                    framesToShow[i] = this->frames[i]->pop();
                    referenceFrameForBlankImage = framesToShow[i];
                } else if (!cameraFirstFrameRead[i]) {
                    framesToShow[i] =
                        referenceFrameForBlankImage.GetBlackImage();
                }
            }

            this->NormalizeNumberOfChannels(framesToShow);

            Frame frame = ImageDisplay::Get().StackImages(
                &framesToShow[0], this->maxFrames, maxHStack);

            if (!this->cfg->resolution.empty()) {
                frame.Resize(this->cfg->resolution);
            } else {
                frame.Resize(this->cfg->scaleFactor, this->cfg->scaleFactor);
            }

            framePublisher.notifySubscribers(std::move(frame));

            std::this_thread::sleep_for(std::chrono::milliseconds(sleepForMs));
        }
    }

    void CamerasFramesBlender::update(int cameraPos, Frame frame) {
        if (frameSubscriberCount > 0) {
            this->frames[cameraPos]->add(frame);
        }
    }

    void CamerasFramesBlender::SetNumberCameras(int total) {
        this->frames.resize(total);

        for (int i = 0; i < total; i++) {
            this->frames[i] = std::make_unique<CircularFIFO<std::mutex>>(5);
        }

        this->maxFrames = total;
    }

    void CamerasFramesBlender::SubscribeToFramesUpdate(
        ISubscriber<Frame>* sub) {
        framePublisher.subscribe(sub);
        frameSubscriberCount++;
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
