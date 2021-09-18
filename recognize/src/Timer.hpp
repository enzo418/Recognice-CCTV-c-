#pragma once

#include <chrono>
namespace Observer
{
    template <class Unit>
    class Timer {
        private:
            std::chrono::time_point<std::chrono::high_resolution_clock> start;

            double duration;

        public:
            Timer(bool startNow = false);
            void Start();

            double GetDuration();

            double GetDurationAndRestart();
    };

} // namespace Observer
