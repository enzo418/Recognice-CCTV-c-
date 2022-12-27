#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>

namespace Observer {
    /**
     * @brief Thread safe fifo queue using mutex
     *
     * @tparam T Queue items type
     */
    template <typename T>
    class BlockingFIFO {
       public:
        BlockingFIFO() {}

        /**
         * @brief Adds a element to the back of queue
         *
         * @param value
         */
        void push_back(T const& value)
            requires std::copy_constructible<T>
        {
            this->mutex.lock();
            this->queue.push_back(value);
            this->mutex.unlock();
        }

        void push_back(T&& value) {
            this->mutex.lock();
            this->queue.push_back(std::move(value));
            this->mutex.unlock();
        }

        /**
         * @brief Return the first element (first in)
         *
         * @return T
         */
        T pop_front() {
            this->mutex.lock();

            T el(std::move(this->queue.front()));

            this->queue.pop_front();

            this->mutex.unlock();

            return el;
        }

        /**
         * @brief Returns the number of elements in the queue.
         *
         * @return size_type
         */
        size_t size() {
            this->mutex.lock();

            const size_t size = this->queue.size();

            this->mutex.unlock();

            return size;
        }

       protected:
        std::mutex mutex;
        std::deque<T> queue;
    };
}  // namespace Observer
