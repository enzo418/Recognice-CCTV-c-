#pragma once

#include <mutex>
#include <condition_variable>

/**
 * @brief Basic c++20 semaphore implementation
 *
 */
class Semaphore {
public:
    Semaphore (int pCount = 0) : count(pCount) {}

    /**
     * @brief increments the internal counter and unblocks acquirers
     * 
     */
    inline void release()
    {
        std::unique_lock<std::mutex> lock(mutex);
        count++;
        cv.notify_one();
    }

    /**
     * @brief decrements the internal counter or blocks until it can
     * 
     */
    inline void acquire()
    {
        std::unique_lock<std::mutex> lock(mutex);

        cv.wait(lock, [this]() { return count > 0; });

        count--;
    }

private:
    std::mutex mutex;
    std::condition_variable cv;
    int count;
};