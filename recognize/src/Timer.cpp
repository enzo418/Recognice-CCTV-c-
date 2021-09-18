#include "Timer.hpp"

namespace Observer
{
    template <class Unit>
    Timer<Unit>::Timer(bool startNow) {
        if (startNow) {
            this->Start();
        }
    }

    template <class Unit>
    void Timer<Unit>::Start() {
        this->start = std::chrono::high_resolution_clock::now();
    }

    template <class Unit>
    double Timer<Unit>::GetDuration() {
        const auto end = std::chrono::high_resolution_clock::now();
        return (end - this->start) / Unit(1);
    }

    template <class Unit>
    double Timer<Unit>::GetDurationAndRestart() {
        const auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = (end - this->start) / Unit(1);
        
        this->start = end;

        return duration;
    }
} // namespace Observer
