#pragma once

#include <cstdint>
namespace Observer {
    struct Size {
        Size(int pWidth, int pHeight)
            : width(pWidth), height(pHeight) {};

        int width;
        int height;
    };
}