#pragma once

#include "BaseObserverPattern.hpp"
#include "Configuration.hpp"
#include "InterfaceFunctionality.hpp"
#include "SimpleBlockingQueue.hpp"

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

            virtual T pop();

            FrameQueue(FrameQueue&&)  noexcept = default;

            virtual ~FrameQueue();

        private:
            using super = SimpleBlockingQueue<T>;
    };

    template<typename T>
    T FrameQueue<T>::pop() {
        super::mutex.lock();

        T elment(std::move(super::queue.back()));

        // if only 1 frames is left, don't delete it
        if (super::queue.size() > 1) {
            super::queue.pop();
        }

        super::mutex.unlock();

        return elment;
    }

    template<typename T>
    void FrameQueue<T>::push(const T &value) {
        SimpleBlockingQueue<T>::push(value);
    }

    template<typename T>
    size_t FrameQueue<T>::size() {
        return SimpleBlockingQueue<T>::size();
    }

    template<typename T>
    FrameQueue<T>::~FrameQueue() {
        ~SimpleBlockingQueue<T>();
    }

    /**
    * @todo write docs
    */
    class FrameDisplay : public FrameEventSubscriber,  public IFunctionality
    {
        public:
            /**
             * @param total Number of frames to display at the same time
             */
            explicit FrameDisplay(int total);

            void Start() override;

            void Stop() override;

            /**
             * @brief add a new frame to the available frames
             *
             * @param framePosition 0 = top left, 1 = top right, ...
             */
            void update(int framePosition, cv::Mat frame) override;

        private:
            std::vector<std::queue<cv::Mat>> frames;
            std::mutex mtxFrames;

            int maxFrames;

            bool running;
    };
}
