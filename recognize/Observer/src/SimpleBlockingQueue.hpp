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
        SimpleBlockingQueue() = default;

        /**
         * @brief Adds a element to the queue
         *
         * @param value
         */
        virtual void push(T const& value);

        /**
         * @brief Get a element from the queue.
         * It assumes there is at least 1 element.
         *
         * @return T
         */
        virtual T pop();

        /**
         * @brief Returns the number of elements in the queue.
         *
         * @return size_type
         */
        virtual size_t size();

       protected:
        std::mutex mutex;
        std::queue<T> queue;
    };

    template <typename T>
    void SimpleBlockingQueue<T>::push(T const& value) {
        this->mutex.lock();
        this->queue.push(value);
        this->mutex.unlock();
    }

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
