#include "Vector.hpp"

namespace Observer {

    double Vec::magnitude() { return this->p1.DistanceTo(this->p2); }

    bool Vec::isNull() {
        return this->p1 == Point(0, 0) && this->p2 == Point(0, 0);
    }
}  // namespace Observer