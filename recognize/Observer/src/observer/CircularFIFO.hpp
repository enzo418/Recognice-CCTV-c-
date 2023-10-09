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

       public:
        CircularFIFO(int size) {
            queue.resize(size);
            front = rear = -1;
            capacity = size;
        }

        /**
         * @brief Check if the queue is full.
         * If it's full the oldest element will be replaced.
         *
         * @return true
         * @return false
         */
        bool full() {
            return (front == 0 && rear == capacity - 1) || (front == rear + 1);
        }

        bool empty() { return front == -1; }

        int size() {
            if (empty()) {
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
            if (full()) {
                // Replace the oldest element if the queue is full
                front = (front + 1) % capacity;
            }

            if (empty()) {
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
            if (empty()) {
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
