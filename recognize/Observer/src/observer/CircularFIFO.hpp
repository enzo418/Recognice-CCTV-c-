#pragma once

#include <vector>

#include "Implementation.hpp"
#include "Lock.hpp"
#include "observer/LockedFIFO.hpp"
#include "observer/Log/log.hpp"

namespace Observer {

    template <typename Lock>
        requires BasicLockable<Lock>
    class CircularFIFO {
       private:
        std::vector<Frame> queue;
        int front, rear, capacity;
        Lock lock;

       public:
        CircularFIFO(int size) {
            queue.resize(size);
            front = rear = -1;
            capacity = size;
        }

       private:
        bool _full() {
            return (front == 0 && rear == capacity - 1) || (front == rear + 1);
        }

        bool _empty() { return front == -1; }

       public:
        /**
         * @brief Check if the queue is full.
         * If it's full the oldest element will be replaced.
         *
         * @return true
         * @return false
         */
        bool full() {
            std::lock_guard<Lock> guard(lock);
            return (front == 0 && rear == capacity - 1) || (front == rear + 1);
        }

        bool empty() {
            std::lock_guard<Lock> guard(lock);
            return front == -1;
        }

        int size() {
            std::lock_guard<Lock> guard(lock);

            if (_empty()) {
                return 0;
            }
            if (rear >= front) {
                return rear - front + 1;
            } else {
                return capacity - (front - rear) + 1;
            }
        }

        /**
         * @brief Add a new element to the queue.
         *
         * @param frame
         */
        void add(Frame& frame) {
            std::lock_guard<Lock> guard(lock);

            if (_full()) {
                // Replace the oldest element if the queue is full
                front = (front + 1) % capacity;
            }

            if (_empty()) {
                front = rear = 0;
            } else {
                rear = (rear + 1) % capacity;
            }

            frame.CopyTo(queue[rear]);
        }

        /**
         * @brief Pop the oldest element from the queue.
         *
         * @return Frame
         */
        Frame pop() {
            std::lock_guard<Lock> guard(lock);

            if (_empty()) {
                throw std::runtime_error("Queue is empty");
            }

            int itemIndex = front;

            if (front == rear) {
                front = rear = -1;
            } else {
                front = (front + 1) % capacity;
            }

            return std::move(queue[itemIndex]);
        }
    };
}  // namespace Observer
