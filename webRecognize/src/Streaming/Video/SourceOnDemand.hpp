#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string_view>

#include "LiveViewStatus.hpp"
#include "observer/Domain/BufferedSource.hpp"
#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Semaphore.hpp"
#include "observer/Timer.hpp"
#include "observer/Utils/SpecialEnums.hpp"

namespace Web::Streaming::Video {

    constexpr double SECONDS_TO_ZOMBIE_ON_DEMAND = 1 * 30;

    class SourceOnDemand : public Observer::Functionality {
       public:
        typedef std::vector<unsigned char> Buffer;
        typedef std::function<void(const SourceOnDemand::Buffer&)>
            EncodedImageCallback;

       public:
        SourceOnDemand(int quality);

        virtual ~SourceOnDemand() = default;

        LiveViewStatus GetStatus();

        /**
         * @brief Run the callback on the frame. If there is no frame, it will
         * return false.
         *
         * @param callback The callback to run on the frame.
         * @return true The callback was run.
         * @return false
         */
        bool RunSafelyOnFrameReady(EncodedImageCallback callback);

        void EnsureOpenAndReady();

       protected:
        /**
         * @brief Return true if it went live and nobody is using its frames.
         *
         * @return true
         * @return false
         */
        bool IsZombie();

        /**
         * @brief Safely set the new frame.
         *
         * @param newFrame
         */
        void SetFrame(Observer::Frame newFrame);

        /**
         * @brief Encode the frame if it wasn't encoded yet.
         * Doing this way, separate from SetFrame, allows the frame to be
         * encoded only when it is needed, and maybe in another thread.
         */
        void EnsureEncoded();

        /**
         * @brief Called when the source should be opened.
         */
        virtual void EnsureOpen() = 0;

       protected:
        LiveViewStatus status;

       private:
        int quality;

        Observer::Frame frame;
        std::mutex mtxFrame;

        std::atomic_flag latestFrameWasEncoded;

        std::atomic_flag firstFrameWasEncoded;

        std::vector<unsigned char> latestImageBuffer;
        std::mutex mtxLatestImageBuffer;

        Observer::Timer<std::chrono::seconds> timerSinceLastRequest;
    };

}  // namespace Web::Streaming::Video