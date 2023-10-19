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
        thread = std::thread(&Functionality::StartWrapper, this);
    }

    void Functionality::Stop() {
        if (!running) return;

        running.store(false, std::memory_order_release);

        if (std::this_thread::get_id() == thread.get_id()) {
            OBSERVER_TRACE("Functionality tried to stop itself.");

            stoppedItself.test_and_set();
        } else {
            stoppedItself.clear();

            OBSERVER_TRACE("Waiting for functionality to stop.");
            if (thread.joinable()) {
                thread.join();
            }

            this->PostStop();
        }

        OBSERVER_TRACE("Functionality stopped.");
    }

    void Functionality::PostStop() {}

    bool Functionality::IsRunning() { return running; }

    void Functionality::StartWrapper() {
        if (!running) return;

        this->InternalStart();

        if (stoppedItself.test()) {
            this->PostStop();
        }
    }
}  // namespace Observer
