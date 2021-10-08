#pragma once

#include <chrono>
namespace Observer
{
    template <class Unit>
    class Timer {
        private:
            std::chrono::time_point<std::chrono::high_resolution_clock> start;

            bool started;
            
        public:
            Timer(bool startNow = false);
            void Start();

            void Stop();

            double GetDuration();

            double GetDurationAndRestart();

            bool Started();
    };

} // namespace Observer
