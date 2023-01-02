#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace Observer {
    /**
     * @brief Very simple thread safe queue using mutex
     *
     * @tparam T Queue items type
     */
    template <typename T>
    class SimpleBlockingQueue {
       public:
        SimpleBlockingQueue();

        /**
         * @brief Adds a element to the queue
         *
         * @param value
         */
        void push(T const& value)
            requires std::copy_constructible<T>
        {
            this->mutex.lock();
            this->queue.push(value);
            this->mutex.unlock();
        }

        void push(T&& value) {
            this->mutex.lock();
            this->queue.push(std::move(value));
            this->mutex.unlock();
        }

        /**
         * @brief Get a element from the queue.
         * It assumes there is at least 1 element.
         *
         * @return T
         */
        T pop();

        /**
         * @brief Returns the number of elements in the queue.
         *
         * @return size_type
         */
        size_t size();

       protected:
        std::mutex mutex;
        std::queue<T> queue;
    };

    template <typename T>
    SimpleBlockingQueue<T>::SimpleBlockingQueue() {}

    template <typename T>
    T SimpleBlockingQueue<T>::pop() {
        this->mutex.lock();

        T elment(std::move(this->queue.back()));

        this->queue.pop();

        this->mutex.unlock();

        return elment;
    }

    template <typename T>
    size_t SimpleBlockingQueue<T>::size() {
        this->mutex.lock();

        const size_t size = this->queue.size();

        this->mutex.unlock();

        return size;
    }
}  // namespace Observer
