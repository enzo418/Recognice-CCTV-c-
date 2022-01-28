#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "../Functionality.hpp"
#include "../Implementation.hpp"
#include "../Log/log.hpp"
#include "../Pattern/Camera/IFrameSubscriber.hpp"
#include "../Pattern/ObserverBasics.hpp"
#include "../Semaphore.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "Configuration/Configuration.hpp"
#include "Configuration/OutputPreviewConfiguration.hpp"

namespace Observer {
    /**
     * @todo Handles the display of frames to the users.
     */
    class FrameDisplay : public ISubscriber<Frame>, public Functionality {
       public:
        FrameDisplay();

        /**
         * @brief Set the number of cameras
         *
         * @param total
         */
        void SetNumberCameras(int total);

        /**
         * @brief Frames to display are given by calling this method.
         *
         * @param frame
         */
        void update(Frame frame) override;

       protected:
        void InternalStart() override;

       private:
        Frame frame;
        std::mutex mutexFrame;
    };
}  // namespace Observer
