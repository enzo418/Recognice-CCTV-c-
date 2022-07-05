#pragma once

#include "observer/Domain/ObserverCentral.hpp"

namespace Web {
    struct RecognizeContext {
        bool running;
        Observer::ObserverCentral* observer;
    };
};  // namespace Web