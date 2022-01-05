#pragma once

#include <string>

namespace Web {
    struct ServerContext {
        std::string rootFolder;
        int port;
    };
}  // namespace Web