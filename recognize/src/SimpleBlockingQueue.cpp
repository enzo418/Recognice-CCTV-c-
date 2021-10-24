#include "SimpleBlockingQueue.hpp"

namespace Observer
{
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
} // namespace Observer
