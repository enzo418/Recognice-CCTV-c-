#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

namespace Observer
{
    /**
     * @brief Very simple thread safe queue using mutex 
     * 
     * @tparam T Queue items type
     */
    template <typename T>
    class SimpleBlockingQueue {
        public:
            /**
             * @brief Adds a element to the queue
             * 
             * @param value 
             */
            virtual void push(T const &value);

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
            size_t size();

        protected:
            std::mutex              mutex;            
            std::queue<T>           queue;
    };
    
} // namespace Observer
