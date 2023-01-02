#pragma once

#include "observer/Domain/ObserverCentral.hpp"

namespace Web {
    struct RecognizeContext {
        bool running;
        std::unique_ptr<Observer::ObserverCentral> observer;
        std::string running_config_id;
    };
};  // namespace Web