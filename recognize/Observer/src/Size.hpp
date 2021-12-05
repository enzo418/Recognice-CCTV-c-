#pragma once

#include <cstdint>
#include <tuple>
namespace Observer {
    struct Size {
        Size() = default;
        Size(int pWidth, int pHeight) : width(pWidth), height(pHeight) {};

        int width;
        int height;

        bool operator==(const Size& other) const {
            return std::tie(other.height, other.width) ==
                   std::tie(height, width);
        };

        bool empty() { return width == 0 || height == 0; }
    };
}  // namespace Observer