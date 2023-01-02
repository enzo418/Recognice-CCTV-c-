#include "Functionality.hpp"

#include <atomic>
#include <thread>

#include "observer/Log/log.hpp"

namespace Observer {

    Functionality::Functionality() { running = false; }

    Functionality::~Functionality() {
        if (running) {
            this->Stop();
        }
    }

    void Functionality::Start() {
        OBSERVER_ASSERT(!running,
                        "Logic error on functionality, it's already running");

        // running = true;
        running.store(true, std::memory_order_release);
        thread = std::thread(&Functionality::InternalStart, this);
    }

    void Functionality::Stop() {
        running = false;

        if (thread.joinable()) {
            thread.join();
        }

        this->PostStop();
    }

    void Functionality::PostStop() {}

    bool Functionality::IsRunning() { return running; }
}  // namespace Observer
