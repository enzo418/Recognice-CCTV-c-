#include <thread>

#include "IFunctionality.hpp"

namespace Observer {

    /**
     * @brief This class is a functionality that needs to run on a
     * separated thread. It handles its own thread life cicle once Start is
     * called.
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

       protected:
        /**
         * @brief Once Start is called a thread will be created with this
         * method. You need to assure that it will only stop from two ways:
         *  1. End of the method is reached, call stop to release the thread
         * (optional).
         *  2. Stop is called. If a inifinite loop was beign used inside this
         * method, a condition will be needed to checked if running is set or
         * not.
         */
        virtual void InternalStart() = 0;

       protected:
        bool running;

       private:
        std::thread thread;
    };
}  // namespace Observer