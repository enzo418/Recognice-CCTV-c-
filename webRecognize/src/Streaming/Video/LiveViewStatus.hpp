#pragma once

#include "../../../../recognize/Observer/vendor/bitmask_operators.hpp"

namespace Web::Streaming::Video {
    enum class LiveViewStatus {
        OPEN = 1,
        CLOSED = 2,
        RUNNING = 4,
        STOPPED = 8,
        ERROR = 16
    };
}  // namespace Web::Streaming::Video

// enable_bitmask_operators -- true -> enable our custom bitmask for our
// enums
template <>
struct enable_bitmask_operators<Web::Streaming::Video::LiveViewStatus> {
    static constexpr bool enable = true;
};