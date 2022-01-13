#pragma once

#include "../../../recognize/Observer/src/Domain/ObserverCentral.hpp"

namespace Web {
    template <typename TFrame>
    struct RecognizeContext {
        bool running;
        Observer::ObserverCentral<TFrame>* observer;
    };
};  // namespace Web