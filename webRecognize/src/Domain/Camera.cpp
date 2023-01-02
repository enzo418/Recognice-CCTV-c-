#include "Camera.hpp"

namespace Web::Domain {
    Camera::Camera(std::string pName, std::string pUri)
        : name(std::move(pName)), uri(pUri) {}
}  // namespace Web::Domain