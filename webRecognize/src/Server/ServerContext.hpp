#pragma once

#include <string>
#include <string_view>

#include "RecognizeContext.hpp"

namespace Web {

    template <bool SSL>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext recognizeContext;
    };
}  // namespace Web