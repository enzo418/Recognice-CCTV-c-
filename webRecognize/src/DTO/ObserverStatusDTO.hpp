#pragma once

#include <optional>
#include <string>

namespace Web {
    struct ObserverStatusDTO {
        bool running;
        std::optional<std::string> config_id;
    };
}  // namespace Web