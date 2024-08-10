#pragma once

#include <string>
#include <string_view>

#include "RecognizeContext.hpp"

namespace Web {
    static std::thread::id g_mainThreadID;

    template <bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext recognizeContext;
    };
}  // namespace Web