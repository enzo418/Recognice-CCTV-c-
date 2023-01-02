#pragma once

#include <mutex>

namespace Observer {

    /**
     * @brief It's a class that provides some ID values, starting from some
     * value. Its main porpoise is to synchronize multiple ID consumers that are
     * in different threads, e.g. EventValidator with GroupID.
     */
    class SynchronizedIDProvider {
       public:
        SynchronizedIDProvider(int initialID);

       public:
        void IncrementBy(int increment);
        void Set(int value);
        int GetCurrent();

        /**
         * @brief Same as calling to IncrementBy and then GetCurrent
         *
         * @param increment
         */
        int GetNext();

       private:
        int value;
        std::mutex mutex;

        typedef std::lock_guard<std::mutex> Guard;
    };
}  // namespace Observer