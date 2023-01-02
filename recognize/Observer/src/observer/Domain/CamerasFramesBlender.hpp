#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "Configuration/Configuration.hpp"
#include "Configuration/OutputPreviewConfiguration.hpp"
#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/Camera/IFrameSubscriber.hpp"
#include "observer/Pattern/ObserverBasics.hpp"
#include "observer/Semaphore.hpp"
#include "observer/SimpleBlockingQueue.hpp"

namespace Observer {
    /**
     * @todo Consumes frames from the camera, combines them into a single image
     * and notifies to all its subscribers.
     */
    class CamerasFramesBlender : public IFrameSubscriber, public Functionality {
       public:
        /**
         * @param total Number of frames to display at the same time
         */
        explicit CamerasFramesBlender(OutputPreviewConfiguration* cfg);

        void SetNumberCameras(int total);

        /**
         * @brief add a new frame to the available frames
         *
         * @param framePosition 0 = top left, 1 = top right, ...
         */
        void update(int framePosition, Frame frame) override;

        void SubscribeToFramesUpdate(ISubscriber<Frame>* sub);

       protected:
        void CalculateSleepTime(int totalNow);

        void InternalStart() override;

        /**
         * @brief If some frames have different number of channels this function
         * will correct that, converting each frame to the maximum number of
         * channel found between them all.
         */
        void NormalizeNumberOfChannels(std::vector<Frame>&);

       private:
        std::vector<std::queue<Frame>> frames;
        std::mutex mtxFrames;

        OutputPreviewConfiguration* cfg;

        int maxFrames;

       private:
        int sleepForMs;

       private:
        Publisher<Frame> framePublisher;
        std::atomic<int> frameSubscriberCount;
    };
}  // namespace Observer
