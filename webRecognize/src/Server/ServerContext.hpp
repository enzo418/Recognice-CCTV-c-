#pragma once

#include <string>

#include "RecognizeContext.hpp"

namespace Web {
    template <typename TFrame>
    struct ServerContext {
        std::string rootFolder;
        int port;

        RecognizeContext<TFrame> recognizeContext;
    };
}  // namespace Web