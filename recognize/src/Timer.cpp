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
        this->started = true;

        this->start = std::chrono::high_resolution_clock::now();
    }

    template <class Unit>
    void Timer<Unit>::Stop() {
        this->started = false;
    }

    template <class Unit>
    double Timer<Unit>::GetDuration() {
        const auto end = std::chrono::high_resolution_clock::now();
        return (end - this->start) / Unit(1);
    }

    template <class Unit>
    double Timer<Unit>::GetDurationAndRestart() {
        auto duration = this->GetDuration();
        
        this->Start();

        return duration;
    }

    template <class Unit>
    bool Timer<Unit>::Started() {
        return this->started;
    }
} // namespace Observer
