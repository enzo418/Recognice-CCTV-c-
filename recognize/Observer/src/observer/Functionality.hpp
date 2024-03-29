#pragma once

#include <atomic>
#include <thread>

#include "IFunctionality.hpp"

namespace Observer {
    /**
     * @brief This class is a functionality that needs to run on a
     * separated thread. It handles its own thread life cycle once Start is
     * called. The result is an easy way to implement multi-threaded
     * functionality.
     */
    class Functionality : public IFunctionality {
       public:
        Functionality();
        ~Functionality();

       public:
        /**
         * @brief Start the functionality.
         * Creates a thread.
         * Will join if it's already running.
         */
        void Start() override final;

        /**
         * @brief Stop the functionality.
         * Joins the thead.
         */
        void Stop() override final;

        bool IsRunning();

       protected:
        void StartWrapper();

        virtual bool CheckCanStart() { return true; }

        /**
         * @brief Once Start is called a thread will be created with this
         * method. You need to assure that it will only stop from two ways:
         *  1. End of the method is reached, call stop to release the thread
         * (optional).
         *  2. Stop is called. If a infinite loop was being used inside this
         * method, a condition will be needed to checked if running is set or
         * not.
         */
        virtual void InternalStart() = 0;

        /**
         * @brief This method is called once Stop is called and the thread is
         * joined. There is no Post/PreStart since there is InternalStart.
         */
        virtual void PostStop();

       protected:
        std::atomic<bool> running;

        /**
         * @brief If the thread calls Stop from itself (IternalStart), it will
         * delay the call to PostStop until the thread is joined if endedFlag is
         * not set. If it is set, it will call PostStop right away.
         */
        std::atomic_flag stoppedItself {false};

       private:
        std::thread thread;
    };
}  // namespace Observer