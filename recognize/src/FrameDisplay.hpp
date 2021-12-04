#pragma once

#include <chrono>
#include <mutex>
#include <thread>

#include "BaseObserverPattern.hpp"
#include "Configuration.hpp"
#include "IFunctionality.hpp"
#include "ImageDisplay.hpp"
#include "ImageTransformation.hpp"
#include "SimpleBlockingQueue.hpp"

namespace Observer {
    template <typename TFrame>
    class FrameEventSubscriber : public ISubscriber<int, TFrame> {
        virtual void update(int camerapos, TFrame frame) = 0;
    };

    template <typename T>
    class FrameQueue : protected SimpleBlockingQueue<T> {
       public:
        virtual void push(T const& value);
        virtual size_t size();

        virtual T pop();

        FrameQueue(FrameQueue&&) noexcept = default;

        virtual ~FrameQueue();

       private:
        using super = SimpleBlockingQueue<T>;
    };

    template <typename T>
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

    template <typename T>
    void FrameQueue<T>::push(const T& value) {
        SimpleBlockingQueue<T>::push(value);
    }

    template <typename T>
    size_t FrameQueue<T>::size() {
        return SimpleBlockingQueue<T>::size();
    }

    template <typename T>
    FrameQueue<T>::~FrameQueue() {
        ~SimpleBlockingQueue<T>();
    }

    /**
     * @todo write docs
     */
    template <typename TFrame>
    class FrameDisplay : public FrameEventSubscriber<TFrame>,
                         public IFunctionality {
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
        void update(int framePosition, TFrame frame) override;

       private:
        std::vector<std::queue<TFrame>> frames;
        std::mutex mtxFrames;

        int maxFrames;

        bool running;
    };

    template <typename TFrame>
    FrameDisplay<TFrame>::FrameDisplay(int total) : maxFrames(total) {
        this->running = false;
        this->frames.reserve(total);
    }

    template <typename TFrame>
    void FrameDisplay<TFrame>::Start() {
        this->running = true;

        ImageDisplay<TFrame>::CreateWindow("images");

        std::vector<TFrame> framesToShow(maxFrames);
        while (this->running) {
            this->mtxFrames.lock();

            for (int i = 0; i < this->maxFrames; i++) {
                framesToShow[i] = this->frames[i].front();
                this->frames[i].pop();
            }

            this->mtxFrames.unlock();

            TFrame show = ImageTransformation<TFrame>::StackImages(
                &framesToShow[0], framesToShow.size());

            ImageDisplay<TFrame>::ShowImage("images", show);

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
}  // namespace Observer
