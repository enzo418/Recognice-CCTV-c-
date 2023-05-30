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

        if (!CheckCanStart()) {
            OBSERVER_WARN("Functionality can't start.");
            return;
        }

        if (thread.joinable()) {  // clean state
            thread.join();
        }

        // running = true;
        running.store(true, std::memory_order_release);
        thread = std::thread(&Functionality::InternalStart, this);
    }

    void Functionality::Stop() {
        OBSERVER_ASSERT(std::this_thread::get_id() != thread.get_id(),
                        "Deadlock detected.");

        if (!running) return;

        running = false;

        if (thread.joinable()) {
            thread.join();
        }

        this->PostStop();
    }

    void Functionality::PostStop() {}

    bool Functionality::IsRunning() { return running; }
}  // namespace Observer
