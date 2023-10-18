#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

namespace Web {
    class ThreadSafeFlag {
       public:
        ThreadSafeFlag() : flag(false) {}

        /**
         * @brief Sets the flag to true and unblocks all threads waiting on it.
         *
         */
        void set() {
            std::unique_lock<std::mutex> lock(mutex);
            flag = true;
            conditionVariable.notify_all();
        }

        /**
         * @brief Sets the flag to false and unblocks all threads waiting on it.
         *
         */
        void reset() {
            std::unique_lock<std::mutex> lock(mutex);
            flag = false;
            conditionVariable.notify_all();
        }

        /**
         * @brief Blocks the thread until the flag is set to true, or the
         * timeout in milliseconds is exceeded.
         *
         * @tparam ms milliseconds to timeout
         * @return true if the flag was set to true
         * @return false if the timeout was exceeded and the flag is still false
         */
        template <int ms>
        bool waitTrue() {
            std::unique_lock<std::mutex> lock(mutex);
            return conditionVariable.wait_for(
                lock, std::chrono::microseconds(ms), [this]() { return flag; });
        }

        /**
         * @brief Blocks the thread until the flag is set to false, or the
         * timeout in milliseconds is exceeded.
         *
         * @tparam ms milliseconds to timeout
         * @return true if the flag was set to false
         * @return false if the timeout was exceeded and the flag is still true
         */
        template <int ms>
        bool waitFalse() {
            std::unique_lock<std::mutex> lock(mutex);
            return conditionVariable.wait_for(lock,
                                              std::chrono::microseconds(ms),
                                              [this]() { return !flag; });
        }

        void lock() { mutex.lock(); }
        void unlock() { mutex.unlock(); }

        // return the value of the flag when used as a bool
        operator bool() const { return flag; }

       private:
        std::mutex mutex;
        std::condition_variable conditionVariable;
        std::atomic<bool> flag;
    };
}  // namespace Web