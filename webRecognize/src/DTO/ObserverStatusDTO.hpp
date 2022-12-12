#pragma once

#include <optional>
#include <string>

struct ObserverStatusDTO {
    bool running;
    std::optional<std::string> config_id;
};