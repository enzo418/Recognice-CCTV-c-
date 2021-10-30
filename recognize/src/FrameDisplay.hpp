#pragma once

#include "BaseObserverPattern.hpp"
#include "Configuration.hpp"

#include <thread>
#include <chrono>
#include <mutex>

namespace Observer {
    class FrameEventSubscriber : public ISubscriber<int, cv::Mat> {
        virtual void update(int camerapos, cv::Mat frame) = 0;
    };

    template <typename T>
    class FrameQueue : protected SimpleBlockingQueue<T> {
        public:
            virtual void push(T const &value);
            virtual size_t size();

            T pop()  {
                super::mutex.lock();

                T elment(std::move(super::queue.back()));

                // if only 1 frames is left, don't delete it
                if (super::queue.size() > 1) {
                    super::queue.pop();
                }

                super::mutex.unlock();

                return elment;
            }

            virtual ~FrameQueue();

        private:
            using super = SimpleBlockingQueue<T>;
    };

    /**
    * @todo write docs
    */
    class FrameDisplay : public FrameEventSubscriber
    {
        public:
            /**
             * @param total Number of frames to display at the same time
             */
            FrameDisplay(int total);

            void Start();

            void Stop();

            /**
             * @brief add a new frame to the available frames
             *
             * @param framePosition 0 = top left, 1 = top right, ...
             */
            void update(int framePosition, cv::Mat frame) override;

        private:
            std::vector<FrameQueue<cv::Mat>> frames;
            std::mutex mtxFrames;

            int maxFrames;

            bool running;
    };
}