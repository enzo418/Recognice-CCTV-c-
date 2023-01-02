#include "Size.hpp"

namespace Observer {
    Size::Size(int pWidth, int pHeight) : width(pWidth), height(pHeight) {}

    bool Size::operator==(const Size& other) const {
        return std::tie(other.height, other.width) == std::tie(height, width);
    }

    bool Size::empty() { return width == 0 || height == 0; }

    std::ostream& operator<<(std::ostream& os, const Size& rt) {
        os << "[" << rt.width << " x " << rt.height << "]";

        return os;
    }
}  // namespace Observer