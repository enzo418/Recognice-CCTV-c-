#include "Functionality.hpp"

#include <thread>

Functionality::Functionality() { running = false; }

Functionality::~Functionality() {
    if (running) {
        this->Stop();
    }
}

void Functionality::Start() {
    if (running) {
        this->Stop();
    }

    running = true;
    thread = std::thread(&Functionality::InternalStart, this);
}

void Functionality::Stop() {
    running = false;

    if (thread.joinable()) {
        thread.join();
    }
}

void Functionality::PostStop() {}

bool Functionality::IsRunning() { return running; }