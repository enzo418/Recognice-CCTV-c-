#pragma once

#include <optional>
#include <string>

#include "observer/Domain/ObserverCentral.hpp"

namespace Web {
    struct ObserverStatusDTO {
        bool running;
        std::optional<std::string> config_id;
        std::vector<Observer::CameraStatus> cameras;
    };
}  // namespace Web