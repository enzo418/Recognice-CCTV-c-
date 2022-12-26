#pragma once

#include <cstdint>
#include <ostream>
#include <tuple>
namespace Observer {
    struct Size {
        Size() = default;
        Size(int pWidth, int pHeight);

        int width;
        int height;

        bool operator==(const Size& other) const;

        bool empty();
    };

    std::ostream& operator<<(std::ostream& os, const Size& rt);
}  // namespace Observer